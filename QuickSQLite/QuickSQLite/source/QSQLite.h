//
//  QSQLite.h
//  QuickSQLite
//
//  Created by sudi on 2016/12/15.
//
//

#ifndef QSQLite_h
#define QSQLite_h

#import "QDBValue.h"
#import "QSQLiteOpenHelper.h"

#define QFormatString(s, ...) ([NSString stringWithFormat:(s), ##__VA_ARGS__ ?:@""])

#define QNumber(x) ((NSNumber*)(x))

#define isNull(x) (x == nil || [x isKindOfClass:[NSNull class]])

#endif /* QSQLite_h */
