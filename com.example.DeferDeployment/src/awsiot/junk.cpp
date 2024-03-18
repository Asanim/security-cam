    std::cout << "defer_update_operation\n";
    auto defer_update_operation = ipc_client_->NewDeferComponentUpdate();
    auto defer_update_request = Aws::Greengrass::DeferComponentUpdateRequest();
    defer_update_request->SetDeploymentId(deployment_id);
    defer_update_request->SetRecheckAfterMs(10000);
    defer_update_request->SetMessage(Aws::Crt::String("Sentinel Vision: Update is not safe, deferring until safe."));

    std::cout << "defer_update_operation->Activate\n";
    auto request_status = defer_update_operation->Activate(*(defer_update_request.get()), nullptr).get();
    std::cout << "Status: " << request_status.StatusToString().c_str();

    std::cout << "defer_update_operation->GetResult\n";
    auto defer_update_result_future = defer_update_operation->GetResult();

    std::cout << "defer_update_result_future.wait_for\n";
    if (defer_update_result_future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout)
    {
      std::cout << "Operation timed out while waiting for response from Greengrass Core." << std::endl;
      exit(-1);
    }

    std::cout << "defer_update_result_future.get\n";
    if (auto defer_update_result = defer_update_result_future.get(); defer_update_result)
    {
      const auto *response = defer_update_result.GetOperationResponse();
      (void)response;
      std::cout << "Successfully delayed update\n";
    }
    else
    {
      auto error_type = defer_update_result.GetResultType();
      if (error_type == OPERATION_ERROR)
      {
        OperationError *error = defer_update_result.GetOperationError();
        if (error->GetMessage().has_value())
        {
          std::cout << "Greengrass Core responded with an error: " << error->GetMessage().value().c_str();
        }
      }
      else
      {
        std::cout << "Attempting to receive the response from the server failed with error code %s\n"
                  << defer_update_result.GetRpcError().StatusToString().c_str();
      }
    }
  }
