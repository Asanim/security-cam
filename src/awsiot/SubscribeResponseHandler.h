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
 *  \file SubscribeResponseHandler.h
 */

#ifndef SUBSCRIBE_RESPONSE_HANDLER_H
#define SUBSCRIBE_RESPONSE_HANDLER_H

#include <iostream>
#include <aws/greengrass/GreengrassCoreIpcModel.h>

class SubscribeResponseHandler : public Aws::Greengrass::SubscribeToTopicStreamHandler {
public:
    virtual ~SubscribeResponseHandler() {}

private:
    void OnStreamEvent(SubscriptionResponseMessage *response) override;
    bool OnStreamError(OperationError *error) override;
    void OnStreamClosed() override;
};

#endif // SUBSCRIBE_RESPONSE_HANDLER_H
