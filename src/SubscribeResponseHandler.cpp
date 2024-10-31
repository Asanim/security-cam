/* 
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 *  
 *  \file SubscribeResponseHandler.cc
 */

#include "SubscribeResponseHandler.h"

void SubscribeResponseHandler::OnStreamEvent(SubscriptionResponseMessage *response) {
    auto jsonMessage = response->GetJsonMessage();
    if (jsonMessage.has_value() && jsonMessage.value().GetMessage().has_value()) {
        auto messageString = jsonMessage.value().GetMessage().value().View().WriteReadable();
        std::cout << "Received JSON message: " << messageString << std::endl;
        // Handle JSON message.
    } else {
        auto binaryMessage = response->GetBinaryMessage();
        if (binaryMessage.has_value() && binaryMessage.value().GetMessage().has_value()) {
            auto messageBytes = binaryMessage.value().GetMessage().value();
            std::string messageString(messageBytes.begin(), messageBytes.end());
            std::cout << "Received binary message: " << messageString << std::endl;
            // Handle binary message.
        }
    }
}

bool SubscribeResponseHandler::OnStreamError(OperationError *error) {
    std::cout << "Stream error: " << std::endl;
    // Handle error.
    return false;  // Return true to close the stream, false to keep the stream open.
}

void SubscribeResponseHandler::OnStreamClosed() {
    std::cout << "Stream closed." << std::endl;
    // Handle close.
}
