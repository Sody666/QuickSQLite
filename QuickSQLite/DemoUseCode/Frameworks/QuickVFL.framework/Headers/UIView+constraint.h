//
//  UIView+constraint.h
//  Quick VFL
//
//  Created by Sou Dai on 16/9/21.
//  Copyright © 2016 Sou Dai. All rights reserved.
//

#import <UIKit/UIKit.h>

#define QVIEW(super, subviewClass) ([super q_addAutolayoutSubviewByClass:[subviewClass class]])

@class QVFLParseResult;
@interface UIView(constraint)
/**
 *  Add a autolayout subview by class;
 *
 *  @param targetClass target widget's class
 *
 *  @return widget generated and added.
 */
-(id)q_addAutolayoutSubviewByClass:(Class)targetClass;

/**
 *  add autolayout constraint to me.
 *
 *  @param text  VFL text
 *  @param views views involed
 *
 *  @return added constraints
 */
-(QVFLParseResult*) q_addConstraintsByText:(NSString*)text
                  involvedViews:(NSDictionary*)views;


/**
 *  Control the visibility of the view.
 *
 *  @param visible   whether show the view
 *  @param vertically whether vertically, or horizontally
 */
-(void)q_setVisibility:(BOOL)visible isVertically:(BOOL)vertically;

/**
 *  Get the visibility of the view for the orientation.
 *
 *  @param vertically whether vertically, or horizontally
 */
-(BOOL)q_visibleVertically:(BOOL)vertically;
@end
