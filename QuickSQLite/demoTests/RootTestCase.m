//
//  LuRootTestCase.m
//  webApp
//
//  Created by sudi on 2016/12/5.
//  Copyright © 2016年 lufax. All rights reserved.
//
#import "RootTestCase.h"

@interface RootTestCase()
@property (nonatomic, strong) NSMutableDictionary* dictTestData;
@end

@implementation RootTestCase

-(void)tearDown{
    
    [self.dictTestData removeAllObjects];
    [super tearDown];
}

-(NSMutableDictionary*)dictTestData{
    if(_dictTestData == nil){
        _dictTestData = [[NSMutableDictionary alloc] init];
    }
    
    return _dictTestData;
}

-(id)dataForKey:(NSString*)key{
    return [self.dictTestData objectForKey:key];
}

-(void)setData:(id)data forKey:(NSString*)key{
    if(data == nil){
        return;
    }
    
    [self.dictTestData setObject:data forKey:key];
}
@end
