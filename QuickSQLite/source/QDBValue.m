//
//  CTContentValue.m
//  SqliteDemo
//
//  Created by Sou Dai on 13-5-29.
//

#import "QDBValue.h"
typedef enum : NSUInteger {
    QDBDataTypeUnknown  = 0,
    QDBDataTypeInteger,
    QDBDataTypeDouble,
    QDBDataTypeText,
    QDBDataTypeBlob,
    
} QDBDataType;

@interface QDBValue ()
@property (nonatomic, strong) id contentData;
@property (nonatomic, copy) NSString *key;
@property (nonatomic, assign) QDBDataType dataType;
@end

@implementation QDBValue
#pragma mark - public methods
-(id)value{
    return self.contentData;
}

+(BOOL)unbindRowIntoValues:(const NSArray *)values fromStatement:(sqlite3_stmt *)stmt{
    int index = 0;
    BOOL result = NO;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        for (QDBValue *value in values) {
            [value unbindValueFromStatment:stmt atIndex:index];
            ++index;
        }
        
        result = YES;
    }
    
    return result;
}

#pragma mark -private methods
+(QDBValue*)instanceForObject:(id)object withKey:(NSString*)key{
    if(key.length == 0 || object == nil){
        return nil;
    }
    
    QDBValue* content = [[QDBValue alloc] init];
    content.contentData = object;
    content.key = [key copy];
    
    if([object isKindOfClass:[NSNumber class]]){
        const char* valueType = ((NSNumber*)object).objCType;
        if(strstr("islqISLQ", valueType) != NULL){
            content.dataType = QDBDataTypeInteger;
        }else if(strstr("fd", valueType) != NULL){
            content.dataType = QDBDataTypeDouble;
        }
    } else if([object isKindOfClass:[NSString class]]){
        content.dataType = QDBDataTypeText;
    } else if([object isKindOfClass:[NSData class]]){
        content.dataType = QDBDataTypeBlob;
    }
    
    

    return content;
}

+(QDBValue*)instanceWithKey:(const NSString*)key{
    if(key.length == 0){
        return nil;
    }
    
    QDBValue* content = [[QDBValue alloc] init];
    content.key = [key copy];
    content.dataType = QDBDataTypeUnknown;
    
    return content;
}



#pragma mark - 数据操作部分

- (void)bindValueIntoStatment:(sqlite3_stmt *)stmt atIndex:(int)index
{
    switch (self.dataType) {
        case QDBDataTypeInteger:
            sqlite3_bind_int(stmt, index, ((NSNumber*)self.contentData).intValue);
            break;
        case QDBDataTypeDouble:
            sqlite3_bind_double(stmt, index, ((NSNumber*)self.contentData).doubleValue);
            break;
        case QDBDataTypeText:{
            const char *text = [self.contentData UTF8String];
            sqlite3_bind_text(stmt, index, text, (int)strlen(text), SQLITE_TRANSIENT);
             break;
        }
        case QDBDataTypeBlob:{
            NSData* data = self.contentData;
            const void *bytes = [data bytes];
            // bug may arise when the data is damn large
            // please don't store large data here
            sqlite3_bind_blob(stmt, index, bytes, (int)data.length, SQLITE_TRANSIENT);
            break;
        }
        default:{
            @throw [NSException exceptionWithName:@"Quick SQLite: Binding Values" reason:@"Unsupported data type" userInfo:nil];
        }
    }
}

- (void)unbindValueFromStatment:(sqlite3_stmt *)stmt atIndex:(int)index
{
    if(self.dataType == QDBDataTypeUnknown){
        switch (sqlite3_column_type(stmt, index)) {
            case SQLITE_INTEGER:
                self.dataType = QDBDataTypeInteger;
                break;
            case SQLITE_FLOAT:
                self.dataType = QDBDataTypeDouble;
                break;
            case SQLITE_BLOB:
                self.dataType = QDBDataTypeBlob;
                break;
            case SQLITE_TEXT:
                self.dataType = QDBDataTypeText;
                break;
        }
    }
    
    switch (self.dataType) {
        case QDBDataTypeInteger:
            self.contentData = [NSNumber numberWithInt:sqlite3_column_int(stmt, index)];
            break;
        case QDBDataTypeDouble:
            self.contentData = [NSNumber numberWithDouble:sqlite3_column_double(stmt, index)];
            break;
        case QDBDataTypeText:
            self.contentData = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, index)];
            break;
        case QDBDataTypeBlob:
            self.contentData = [NSData dataWithBytes:sqlite3_column_blob(stmt, index) length:sqlite3_column_bytes(stmt, index)];
            break;
        default:{
            @throw [NSException exceptionWithName:@"Quick Sqlite: Unbinding Values." reason:@"Unsupported data type" userInfo:nil];
        }
    }
}
@end

@interface QDBValue(helper)
/**
 Prepare an array of QDBValues with pairs of key and value.
 
 @param keyValues key and value for column name and value
 @return prepared values
 */
+(NSArray*)valuesWithDictionary:(const NSDictionary*)keyValues;

/**
 Mapping the content value for query string.
 Note: if you don't need some kinds of format, just provide nil.
 for example, set updateOutput and insertOutput to nil if you want
 query format only.
 
 @param contentValues values
 @param queryOutput as name hints
 @param updateOutput as name hints
 @param insertOutput as name hints
 */
+(void)generateSQLWithValues:(const NSArray<QDBValue*>*)contentValues
                       query:(NSString**)queryOutput
                      update:(NSString**)updateOutput
                      insert:(NSString**)insertOutput;



/**
 *  unbind values from statement.
 *  For inner use.
 *
 *  @param values value to bind to
 *  @param stmt   value to bind from
 *  @return whether row of data. nil if failed.
 */
+(NSDictionary*)unbindRowIntoDictionaryWithValues:(const NSArray<QDBValue*> *)values
                                    fromStatement:(sqlite3_stmt *)stmt;


/**
 *  Bind values to statement.
 *
 *  @param values value to bind to
 *  @param stmt   value to bind from
 */
+(void)bindRowWithValues:(const NSArray *)values intoStatement:(sqlite3_stmt *)stmt;
@end

@implementation QDBValue (helper)
+(NSArray*)valuesWithDictionary:(const NSDictionary*)keyValues{
    NSMutableArray* result = [[NSMutableArray alloc] initWithCapacity:keyValues.count];
    for (NSString* key in keyValues) {
        [result addObject:[self instanceForObject:keyValues[key] withKey:key]];
    }
    
    return [result copy];
}

+(void)generateSQLWithValues:(const NSArray*)contentValues
                       query:(NSString**)queryOutput
                      update:(NSString**)updateOutput
                      insert:(NSString**)insertOutput{
    NSMutableString* query = [[NSMutableString alloc] init];
    NSMutableString* updateMap = [[NSMutableString alloc] init];
    NSMutableString* insertMap = [[NSMutableString alloc] init];
    
    NSString* comma = @"";
    for (QDBValue* value in contentValues) {
        [query appendFormat:@"%@%@", comma, value.key];
        [updateMap appendFormat:@"%@%@=?", comma, value.key];
        [insertMap appendFormat:@"%@?", comma];
        
        comma = @", ";
    }
    
    if(queryOutput != nil){
        *queryOutput = [query copy];
    }
    
    if(updateOutput != nil){
        *updateOutput = [updateMap copy];
    }
    
    if(insertOutput != nil){
        *insertOutput = [insertMap copy];
    }
}

+(NSDictionary*)unbindRowIntoDictionaryWithValues:(const NSArray *)values fromStatement:(sqlite3_stmt *)stmt{
    if(![self unbindRowIntoValues:values fromStatement:stmt]){
        return nil;
    }
    
    NSMutableDictionary* result = [[NSMutableDictionary alloc] init];
    for (QDBValue* column in values) {
        [result setObject:column.value forKey:column.key];
    }
    
    return result;
}

+(void)bindRowWithValues:(const NSArray *)values intoStatement:(sqlite3_stmt *)stmt
{
    int index = 0;
    
    for (QDBValue *value in values) {
        ++index;
        [value bindValueIntoStatment:stmt atIndex:index];
    }
}
@end
