/*=====================================================================
AdminHandlers.cpp
-----------------
Copyright Glare Technologies Limited 2021 -
=====================================================================*/
#include "AdminHandlers.h"


#include "RequestInfo.h"
#include "Response.h"
#include "Escaping.h"
#include "ResponseUtils.h"
#include "WebServerResponseUtils.h"
#include "LoginHandlers.h"
#include "../server/ServerWorldState.h"
#include <ConPrint.h>
#include <Exception.h>
#include <Lock.h>
#include <Parser.h>
#include <Escaping.h>


namespace AdminHandlers
{


std::string sharedAdminHeader(ServerAllWorldsState& world_state, const web::RequestInfo& request_info)
{
	std::string page_out = WebServerResponseUtils::standardHeader(world_state, request_info, /*page title=*/"Admin");

	page_out += "<p><a href=\"/admin\">Main admin page</a> | <a href=\"/admin_users\">Users</a> | <a href=\"/admin_parcels\">Parcels</a> | ";
	page_out += "<a href=\"/admin_parcel_auctions\">Parcel Auctions</a> | <a href=\"/admin_orders\">Orders</a> | <a href=\"/admin_sub_eth_transactions\">Eth Transactions</a></p>";

	return page_out;
}


void renderMainAdminPage(ServerAllWorldsState& world_state, const web::RequestInfo& request_info, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request_info))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	std::string page_out = sharedAdminHeader(world_state, request_info);

	page_out += "Welcome!";

	web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, page_out);
}


void renderUsersPage(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	std::string page_out = sharedAdminHeader(world_state, request);

	{ // Lock scope
		Lock lock(world_state.mutex);

		// Print out users
		page_out += "<h2>Users</h2>\n";

		for(auto it = world_state.user_id_to_users.begin(); it != world_state.user_id_to_users.end(); ++it)
		{
			const User* user = it->second.ptr();
			page_out += "<div>\n";
			page_out += "id: " + user->id.toString() + ",       username: " + web::Escaping::HTMLEscape(user->name) + ",       email: " + web::Escaping::HTMLEscape(user->email_address) + ",      joined " + user->created_time.timeAgoDescription() +
				"  linked eth address: <span style=\"color: grey;\">" + user->controlled_eth_address + "</span>";
			page_out += "</div>\n";
		}

		/*page_out += "<table>";
		for(auto it = world_state.user_id_to_users.begin(); it != world_state.user_id_to_users.end(); ++it)
		{
		const User* user = it->second.ptr();
		page_out += "<tr>\n";
		page_out += "<td>" + user->id.toString() + "</td><td>" + web::Escaping::HTMLEscape(user->name) + "</td><td>" + web::Escaping::HTMLEscape(user->email_address) + "</td><td>" + user->created_time.timeAgoDescription() + "</td>";
		page_out += "</tr>\n";
		}
		page_out += "</table>";*/
	} // End Lock scope

	web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, page_out);
}


void renderParcelsPage(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	std::string page_out = sharedAdminHeader(world_state, request);

	{ // Lock scope
		Lock lock(world_state.mutex);

		page_out += "<h2>Root world Parcels</h2>\n";

		Reference<ServerWorldState> root_world = world_state.getRootWorldState();

		for(auto it = root_world->parcels.begin(); it != root_world->parcels.end(); ++it)
		{
			const Parcel* parcel = it->second.ptr();

			// Look up owner
			std::string owner_username;
			auto user_res = world_state.user_id_to_users.find(parcel->owner_id);
			if(user_res == world_state.user_id_to_users.end())
				owner_username = "[No user found]";
			else
				owner_username = user_res->second->name;

			page_out += "<p>\n";
			page_out += "<a href=\"/parcel/" + parcel->id.toString() + "\">Parcel " + parcel->id.toString() + "</a><br/>" +
				"owner: " + web::Escaping::HTMLEscape(owner_username) + "<br/>" +
				"description: " + web::Escaping::HTMLEscape(parcel->description) + "<br/>" +
				"created " + parcel->created_time.timeAgoDescription();

			// Get any auctions for parcel
			page_out += "<div>    \n";
			for(size_t i=0; i<parcel->parcel_auction_ids.size(); ++i)
			{
				const uint32 auction_id = parcel->parcel_auction_ids[i];
				auto auction_res = world_state.parcel_auctions.find(auction_id);
				if(auction_res != world_state.parcel_auctions.end())
				{
					const ParcelAuction* auction = auction_res->second.ptr();
					if(auction->auction_state == ParcelAuction::AuctionState_ForSale)
						page_out += " <a href=\"/parcel_auction/" + toString(auction->id) + "\">Auction " + toString(auction->id) + ": For sale</a><br/>";
					else if(auction->auction_state == ParcelAuction::AuctionState_Sold)
						page_out += " <a href=\"/parcel_auction/" + toString(auction->id) + "\">Auction " + toString(auction->id) + ": Parcel sold.</a><br/>";
				}
			}
			page_out += "</div>    \n";

			page_out += " <a href=\"/admin_create_parcel_auction/" + parcel->id.toString() + "\">Create auction</a>";

			page_out += "</p>\n";
			page_out += "<br/>  \n";
		}
	} // End Lock scope

	web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, page_out);
}


void renderParcelAuctionsPage(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	std::string page_out = sharedAdminHeader(world_state, request);

	{ // Lock scope
		Lock lock(world_state.mutex);

		page_out += "<h2>Parcel auctions</h2>\n";

		for(auto it = world_state.parcel_auctions.begin(); it != world_state.parcel_auctions.end(); ++it)
		{
			const ParcelAuction* auction = it->second.ptr();

			page_out += "<p>\n";
			page_out += "<a href=\"/parcel_auction/" + toString(auction->id) + "\">Parcel Auction " + toString(auction->id) + "</a><br/>" +
				"parcel: <a href=\"/parcel/" + auction->parcel_id.toString() + "\">" + auction->parcel_id.toString() + "</a><br/>" + 
				"state: ";

			if(auction->auction_state == ParcelAuction::AuctionState_ForSale)
			{
				page_out += "for-sale";
				if(!auction->currentlyForSale())
					page_out += " [Expired]";
			}
			else if(auction->auction_state == ParcelAuction::AuctionState_Sold)
				page_out += "sold";
			//else if(auction->auction_state == ParcelAuction::AuctionState_NotSold)
			//	page_out += "not-sold";
			page_out += "<br/>";

			page_out += 
				"start time: " + auction->auction_start_time.RFC822FormatedString() + "(" + auction->auction_start_time.timeDescription() + ")<br/>" + 
				"end time: " + auction->auction_end_time.RFC822FormatedString() + "(" + auction->auction_end_time.timeDescription() + ")<br/>" +
				"start price: " + toString(auction->auction_start_price) + ", end price: " + toString(auction->auction_end_price) + "<br/>" +
				"sold_price: " + toString(auction->sold_price) + "<br/>" +
				"sold time: " + auction->auction_sold_time.RFC822FormatedString() + "(" + auction->auction_sold_time.timeDescription() + ")<br/>" +
				"order#: <a href=\"/admin_order/" + toString(auction->order_id) + "\">" + toString(auction->order_id) + "</a>";

			page_out += "</p>\n";
		}
	} // End Lock scope

	web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, page_out);
}


void renderOrdersPage(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	std::string page_out = sharedAdminHeader(world_state, request);

	{ // Lock scope
		Lock lock(world_state.mutex);

		page_out += "<h2>Orders</h2>\n";

		for(auto it = world_state.orders.begin(); it != world_state.orders.end(); ++it)
		{
			const Order* order = it->second.ptr();

			// Look up user who made the order
			std::string orderer_username;
			auto user_res = world_state.user_id_to_users.find(order->user_id);
			if(user_res == world_state.user_id_to_users.end())
				orderer_username = "[No user found]";
			else
				orderer_username = user_res->second->name;


			page_out += "<p>\n";
			page_out += "<a href=\"/admin_order/" + toString(order->id) + "\">Order " + toString(order->id) + "</a>, " +
				"orderer: " + web::Escaping::HTMLEscape(orderer_username) + "<br/>" +
				"parcel: <a href=\"/parcel/" + order->parcel_id.toString() + "\">" + order->parcel_id.toString() + "</a>, " + "<br/>" +
				"created_time: " + order->created_time.RFC822FormatedString() + "(" + order->created_time.timeAgoDescription() + ")<br/>" +
				"payer_email: " + web::Escaping::HTMLEscape(order->payer_email) + "<br/>" +
				"gross_payment: " + ::toString(order->gross_payment) + "<br/>" +
				"paypal_data: " + web::Escaping::HTMLEscape(order->paypal_data.substr(0, 60)) + "...</br>" +
				"coinbase charge code: " + order->coinbase_charge_code + "</br>" +
				"coinbase charge status: " + order->coinbase_status + "</br>" +
				"confirmed: " + boolToString(order->confirmed);

			page_out += "</p>    \n";
		}
	} // End Lock scope

	web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, page_out);
}


void renderSubEthTransactionsPage(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	std::string page_out = sharedAdminHeader(world_state, request);

	{ // Lock scope
		Lock lock(world_state.mutex);

		page_out += "<h2>Substrata Ethereum Transactions</h2>\n";

		for(auto it = world_state.sub_eth_transactions.begin(); it != world_state.sub_eth_transactions.end(); ++it)
		{
			const SubEthTransaction* trans = it->second.ptr();

			// Look up user who initiated the transaction
			std::string username;
			auto user_res = world_state.user_id_to_users.find(trans->initiating_user_id);
			if(user_res == world_state.user_id_to_users.end())
				username = "[No user found]";
			else
				username = user_res->second->name;


			page_out += "<p>\n";
			page_out += "Transaction " + toString(trans->id) + ", " +
				"initiating user: " + web::Escaping::HTMLEscape(username) + "<br/>" +
				"user_eth_address: " + web::Escaping::HTMLEscape(trans->user_eth_address) + "<br/>" +
				"parcel: <a href=\"/parcel/" + trans->parcel_id.toString() + "\">" + trans->parcel_id.toString() + "</a>, " + "<br/>" +
				"created_time: " + trans->created_time.RFC822FormatedString() + "(" + trans->created_time.timeAgoDescription() + ")<br/>" +
				"state: " + web::Escaping::HTMLEscape(SubEthTransaction::statestring(trans->state)) + "<br/>";
			if(trans->state != SubEthTransaction::State_New)
			{
				page_out += "submitted_time: " + trans->submitted_time.RFC822FormatedString() + "(" + trans->created_time.timeAgoDescription() + ")<br/>";
				page_out += "txn hash: " + web::Escaping::HTMLEscape(trans->transaction_hash.toHexString()) + "<br/>";
				page_out += "error msg: " + web::Escaping::HTMLEscape(trans->submission_error_message) + "<br/>";
			}

			page_out +=
				"nonce: " + toString(trans->nonce) + "<br/>";

			page_out += "</p>    \n";
		}
	} // End Lock scope

	web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, page_out);
}


void renderAdminOrderPage(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	// Parse order id from request path
	Parser parser(request.path.c_str(), request.path.size());
	if(!parser.parseString("/admin_order/"))
		throw glare::Exception("Failed to parse /admin_order/");

	uint32 order_id;
	if(!parser.parseUnsignedInt(order_id))
		throw glare::Exception("Failed to parse order id");


	std::string page_out = sharedAdminHeader(world_state, request);

	{ // Lock scope
		Lock lock(world_state.mutex);

		page_out += "<h2>Order " + toString(order_id) + "</h2>\n";

		auto res = world_state.orders.find(order_id);
		if(res == world_state.orders.end())
		{
			page_out += "No order with that id found.";
		}
		else
		{
			const Order* order = res->second.ptr();

			// Look up user who made the order
			std::string orderer_username;
			auto user_res = world_state.user_id_to_users.find(order->user_id);
			if(user_res == world_state.user_id_to_users.end())
				orderer_username = "[No user found]";
			else
				orderer_username = user_res->second->name;


			page_out += "<p>\n";
			page_out += "<a href=\"/admin_order/" + toString(order->id) + "\">Order " + toString(order->id) + "</a>, " +
				"orderer: " + web::Escaping::HTMLEscape(orderer_username) + "<br/>" +
				"parcel: <a href=\"/parcel/" + order->parcel_id.toString() + "\">" + order->parcel_id.toString() + "</a>, " + "<br/>" +
				"created_time: " + order->created_time.RFC822FormatedString() + "(" + order->created_time.timeAgoDescription() + ")<br/>" +
				"payer_email: " + web::Escaping::HTMLEscape(order->payer_email) + "<br/>" +
				"gross_payment: " + ::toString(order->gross_payment) + "<br/>" +
				"currency: " + web::Escaping::HTMLEscape(order->currency) + "<br/>" +
				"paypal/coinbase data: " + web::Escaping::HTMLEscape(order->paypal_data) + "</br>" +
				"coinbase charge code: " + order->coinbase_charge_code + "</br>" +
				"coinbase charge status: " + order->coinbase_status + "</br>" +
				"confirmed: " + boolToString(order->confirmed);

			page_out += "</p>    \n";
		}
	} // End Lock scope

	web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, page_out);
}



void renderCreateParcelAuction(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	// Parse parcel id from request path
	Parser parser(request.path.c_str(), request.path.size());
	if(!parser.parseString("/admin_create_parcel_auction/"))
		throw glare::Exception("Failed to parse /admin_create_parcel_auction/");

	uint32 id;
	if(!parser.parseUnsignedInt(id))
		throw glare::Exception("Failed to parse parcel id");
	const ParcelID parcel_id(id);


	std::string page_out = WebServerResponseUtils::standardHTMLHeader(request, "Sign Up");

	page_out += "<body>";
	page_out += "</head><h1>Create Parcel Auction</h1><body>";

	page_out += "<form action=\"/admin_create_parcel_auction_post\" method=\"post\">";
	page_out += "parcel id: <input type=\"number\" name=\"parcel_id\" value=\"" + parcel_id.toString() + "\"><br>";
	page_out += "auction start time: <input type=\"number\" name=\"auction_start_time\"   value=\"0\"> hours from now<br>";
	page_out += "auction end time:   <input type=\"number\" name=\"auction_end_time\"     value=\"72\"> hours from now<br>";
	page_out += "auction start price: <input type=\"number\" step=\"0.01\" name=\"auction_start_price\" value=\"1000\"> EUR<br/>";
	page_out += "auction end price: <input type=\"number\" step=\"0.01\" name=\"auction_end_price\"     value=\"50\"> EUR<br/>";
	page_out += "<input type=\"submit\" value=\"Create auction\">";
	page_out += "</form>";

	web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, page_out);
}


void createParcelAuctionPost(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	try
	{
		// Try and log in user
		const uint64 parcel_id              = stringToUInt64(request.getPostField("parcel_id").str());
		const double auction_start_time_hrs = stringToDouble(request.getPostField("auction_start_time").str());
		const double auction_end_time_hrs   = stringToDouble(request.getPostField("auction_end_time").str());
		const double auction_start_price    = stringToDouble(request.getPostField("auction_start_price").str());
		const double auction_end_price      = stringToDouble(request.getPostField("auction_end_price").str());

		{ // Lock scope

			Lock lock(world_state.mutex);

			// Lookup parcel
			const auto res = world_state.getRootWorldState()->parcels.find(ParcelID((uint32)parcel_id));
			if(res != world_state.getRootWorldState()->parcels.end())
			{
				// Found user for username
				Parcel* parcel = res->second.ptr();

				ParcelAuctionRef auction = new ParcelAuction();
				auction->id = (uint32)world_state.parcel_auctions.size() + 1;
				auction->parcel_id = parcel->id;
				auction->auction_state = ParcelAuction::AuctionState_ForSale;
				auction->auction_start_time  = TimeStamp((uint64)(TimeStamp::currentTime().time + auction_start_time_hrs * 3600));
				auction->auction_end_time    = TimeStamp((uint64)(TimeStamp::currentTime().time + auction_end_time_hrs   * 3600));
				auction->auction_start_price = auction_start_price;
				auction->auction_end_price   = auction_end_price;

				world_state.parcel_auctions[auction->id] = auction;

				// Make new screenshot (request) for parcel auction

				//TEMP: scan over all screenshots and find highest used ID. (was running into a problem on localhost of id >= num items)
				uint64 highest_id = 0;
				for(auto it = world_state.screenshots.begin(); it != world_state.screenshots.end(); ++it)
					highest_id = myMax(highest_id, it->first);

				// Close-in screenshot
				{
					ScreenshotRef shot = new Screenshot();
					shot->id = highest_id + 1;
					parcel->getScreenShotPosAndAngles(shot->cam_pos, shot->cam_angles);
					shot->width_px = 650;
					shot->highlight_parcel_id = (int)parcel_id;
					shot->created_time = TimeStamp::currentTime();
					shot->state = Screenshot::ScreenshotState_notdone;

					world_state.screenshots[shot->id] = shot;

					auction->screenshot_ids.push_back(shot->id);
				}
				// Zoomed-out screenshot
				{
					ScreenshotRef shot = new Screenshot();
					shot->id = highest_id + 2;
					parcel->getFarScreenShotPosAndAngles(shot->cam_pos, shot->cam_angles);
					shot->width_px = 650;
					shot->highlight_parcel_id = (int)parcel_id;
					shot->created_time = TimeStamp::currentTime();
					shot->state = Screenshot::ScreenshotState_notdone;

					world_state.screenshots[shot->id] = shot;

					auction->screenshot_ids.push_back(shot->id);
				}

				
				conPrint("Created screenshot for auction");
				
				parcel->parcel_auction_ids.push_back(auction->id);

				world_state.markAsChanged();

				web::ResponseUtils::writeRedirectTo(reply_info, "/parcel_auction/" + toString(auction->id));
			}
		} // End lock scope

	}
	catch(glare::Exception& e)
	{
		conPrint("handleLoginPost error: " + e.what());
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Error: " + e.what());
	}
}


void renderSetParcelOwnerPage(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	// Parse parcel id from request path
	Parser parser(request.path.c_str(), request.path.size());
	if(!parser.parseString("/admin_set_parcel_owner/"))
		throw glare::Exception("Failed to parse /admin_set_parcel_owner/");

	uint32 id;
	if(!parser.parseUnsignedInt(id))
		throw glare::Exception("Failed to parse parcel id");
	const ParcelID parcel_id(id);


	std::string page_out = WebServerResponseUtils::standardHTMLHeader(request, "Sign Up");

	page_out += "<body>";
	page_out += "</head><h1>Set parcel owner</h1><body>";

	{ // Lock scope

		Lock lock(world_state.mutex);

		// Lookup parcel
		const auto res = world_state.getRootWorldState()->parcels.find(parcel_id);
		if(res != world_state.getRootWorldState()->parcels.end())
		{
			// Found user for username
			Parcel* parcel = res->second.ptr();

			// Look up owner
			std::string owner_username;
			auto user_res = world_state.user_id_to_users.find(parcel->owner_id);
			if(user_res == world_state.user_id_to_users.end())
				owner_username = "[No user found]";
			else
				owner_username = user_res->second->name;

			page_out += "<p>Current owner: " + web::Escaping::HTMLEscape(owner_username) + " (user id: " + parcel->owner_id.toString() + ")</p>   \n";

			page_out += "<form action=\"/admin_set_parcel_owner_post\" method=\"post\">";
			page_out += "parcel id: <input type=\"number\" name=\"parcel_id\" value=\"" + parcel_id.toString() + "\"><br>";
			page_out += "new owner id: <input type=\"number\" name=\"new_owner_id\" value=\"" + parcel->owner_id.toString() + "\"><br>";
			page_out += "<input type=\"submit\" value=\"Change owner\">";
			page_out += "</form>";
		}
	} // End lock scope

	web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, page_out);
}


void handleSetParcelOwnerPost(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	try
	{
		const int parcel_id    = request.getPostIntField("parcel_id");
		const int new_owner_id = request.getPostIntField("new_owner_id");

		{ // Lock scope

			Lock lock(world_state.mutex);

			// Lookup parcel
			const auto res = world_state.getRootWorldState()->parcels.find(ParcelID((uint32)parcel_id));
			if(res != world_state.getRootWorldState()->parcels.end())
			{
				// Found user for username
				Parcel* parcel = res->second.ptr();

				parcel->owner_id = UserID(new_owner_id);

				// Set parcel admins and writers to the new user as well.
				parcel->admin_ids  = std::vector<UserID>(1, UserID(new_owner_id));
				parcel->writer_ids = std::vector<UserID>(1, UserID(new_owner_id));

				world_state.denormaliseData(); // Update denormalised data which includes parcel owner name

				world_state.markAsChanged();

				web::ResponseUtils::writeRedirectTo(reply_info, "/parcel/" + toString(parcel_id));
			}
		} // End lock scope
	}
	catch(glare::Exception& e)
	{
		conPrint("handleSetParcelOwnerPost error: " + e.what());
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Error: " + e.what());
	}
}


void handleMarkParcelAsNFTMintedPost(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	try
	{
		const int parcel_id = request.getPostIntField("parcel_id");

		{ // Lock scope

			Lock lock(world_state.mutex);

			// Lookup parcel
			const auto res = world_state.getRootWorldState()->parcels.find(ParcelID((uint32)parcel_id));
			if(res != world_state.getRootWorldState()->parcels.end())
			{
				Parcel* parcel = res->second.ptr();
				parcel->nft_status = Parcel::NFTStatus_MintedNFT;

				world_state.markAsChanged();

				web::ResponseUtils::writeRedirectTo(reply_info, "/parcel/" + toString(parcel_id));
			}
		} // End lock scope
	}
	catch(glare::Exception& e)
	{
		conPrint("handleMarkParcelAsNFTMintedPost error: " + e.what());
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Error: " + e.what());
	}
}


// Creates a new minting transaction for the parcel.
void handleRetryParcelMintPost(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	try
	{
		const int parcel_id = request.getPostIntField("parcel_id");

		{ // Lock scope
			Lock lock(world_state.mutex);

			User* logged_in_user = LoginHandlers::getLoggedInUser(world_state, request);

			// Lookup parcel
			const auto res = world_state.getRootWorldState()->parcels.find(ParcelID((uint32)parcel_id));
			if(res != world_state.getRootWorldState()->parcels.end())
			{
				Parcel* parcel = res->second.ptr();

				// Make an Eth transaction to mint the parcel
				SubEthTransactionRef transaction = new SubEthTransaction();
				transaction->id = world_state.getNextSubEthTransactionUID();
				transaction->created_time = TimeStamp::currentTime();
				transaction->state = SubEthTransaction::State_New;
				transaction->initiating_user_id = logged_in_user->id;
				transaction->parcel_id = parcel->id;
				transaction->user_eth_address = logged_in_user->controlled_eth_address;

				parcel->minting_transaction_id = transaction->id;

				world_state.sub_eth_transactions[transaction->id] = transaction;

				world_state.markAsChanged();

				web::ResponseUtils::writeRedirectTo(reply_info, "/admin_sub_eth_transactions");
			}
		} // End lock scope
	}
	catch(glare::Exception& e)
	{
		conPrint("handleMarkParcelAsNFTMintedPost error: " + e.what());
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Error: " + e.what());
	}
}


void handleRegenerateParcelAuctionScreenshots(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	try
	{
		const int parcel_auction_id = request.getPostIntField("parcel_auction_id");

		{ // Lock scope

			Lock lock(world_state.mutex);

			// Lookup parcel auction
			const auto res = world_state.parcel_auctions.find(parcel_auction_id);
			if(res != world_state.parcel_auctions.end())
			{
				ParcelAuction* auction = res->second.ptr();

				for(size_t z=0; z<auction->screenshot_ids.size(); ++z)
				{
					const uint64 screenshot_id = auction->screenshot_ids[z];

					auto shot_res = world_state.screenshots.find(screenshot_id);
					if(shot_res != world_state.screenshots.end())
					{
						Screenshot* shot = shot_res->second.ptr();
						shot->state = Screenshot::ScreenshotState_notdone;
					}
				}

				world_state.markAsChanged();
			}
		} // End lock scope

		web::ResponseUtils::writeRedirectTo(reply_info, "/parcel_auction/" + toString(parcel_auction_id));
	}
	catch(glare::Exception& e)
	{
		conPrint("handleRegenerateParcelAuctionScreenshots error: " + e.what());
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Error: " + e.what());
	}
}


void handleRegenerateParcelScreenshots(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	try
	{
		const ParcelID parcel_id = ParcelID(request.getPostIntField("parcel_id"));

		{ // Lock scope

			Lock lock(world_state.mutex);

			// Lookup parcel
			const auto res = world_state.getRootWorldState()->parcels.find(parcel_id);
			if(res != world_state.getRootWorldState()->parcels.end())
			{
				Parcel* parcel = res->second.ptr();

				for(size_t z=0; z<parcel->screenshot_ids.size(); ++z)
				{
					const uint64 screenshot_id = parcel->screenshot_ids[z];

					auto shot_res = world_state.screenshots.find(screenshot_id);
					if(shot_res != world_state.screenshots.end())
					{
						Screenshot* shot = shot_res->second.ptr();
						shot->state = Screenshot::ScreenshotState_notdone;
					}
				}

				world_state.markAsChanged();
			}
		} // End lock scope

		web::ResponseUtils::writeRedirectTo(reply_info, "/parcel/" + parcel_id.toString());
	}
	catch(glare::Exception& e)
	{
		conPrint("handleRegenerateParcelScreenshots error: " + e.what());
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Error: " + e.what());
	}
}


void handleTerminateParcelAuction(ServerAllWorldsState& world_state, const web::RequestInfo& request, web::ReplyInfo& reply_info)
{
	if(!LoginHandlers::loggedInUserHasAdminPrivs(world_state, request))
	{
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Access denied sorry.");
		return;
	}

	try
	{
		const int parcel_auction_id = request.getPostIntField("parcel_auction_id");

		{ // Lock scope

			Lock lock(world_state.mutex);

			// Lookup parcel auction
			const auto res = world_state.parcel_auctions.find(parcel_auction_id);
			if(res != world_state.parcel_auctions.end())
			{
				ParcelAuction* auction = res->second.ptr();

				auction->auction_end_time = TimeStamp::currentTime(); // Just mark the end time as now

				world_state.markAsChanged();
			}
		} // End lock scope

		web::ResponseUtils::writeRedirectTo(reply_info, "/parcel_auction/" + toString(parcel_auction_id));
	}
	catch(glare::Exception& e)
	{
		conPrint("handleLoginPost error: " + e.what());
		web::ResponseUtils::writeHTTPOKHeaderAndData(reply_info, "Error: " + e.what());
	}
}


} // end namespace AdminHandlers
