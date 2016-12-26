//
//  LuRootTestCase.h
//  webApp
//
//  Created by sudi on 2016/12/5.
//  Copyright © 2016年 lufax. All rights reserved.
//

#import <XCTest/XCTest.h>

#define BindUTData(...) do {\
    NSDictionary* allData = NSDictionaryOfVariableBindings(__VA_ARGS__); \
    if(allData.count > 0){\
        for(NSString* key in allData.allKeys){\
            [self setData:allData[key] forKey:key];\
        }\
    }\
}while(0)

#define TData(key) ([self dataForKey:(key)])

@interface RootTestCase : XCTestCase
-(id)dataForKey:(NSString*)key;
-(void)setData:(id)data forKey:(NSString*)key;
@end
