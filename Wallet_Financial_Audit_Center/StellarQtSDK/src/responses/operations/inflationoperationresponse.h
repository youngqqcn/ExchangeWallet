#ifndef INFLATIONOPERATIONRESPONSE_H
#define INFLATIONOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
/**
 * Represents Inflation operation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class InflationOperationResponse : public OperationResponse
{
    Q_OBJECT
public:
    InflationOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~InflationOperationResponse();
};
Q_DECLARE_METATYPE(InflationOperationResponse*)
#endif // INFLATIONOPERATIONRESPONSE_H
