// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AuraHUD.h"

#include "UI/WidgetController/AttributeMenuWidgetController.h"


/**
 * ��ȡ���ǲ�ؼ�������ʵ��
 * 
 * @param WCParams �ؼ������������ṹ�壬������ҿ����������״̬������ϵͳ��������Լ�����Ϣ
 * @return ����UOverlayWidgetController���͵Ŀؼ�������ʵ��ָ��
 */
UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
    // ����ؼ���������δ�������򴴽��µ�ʵ�������ò���
    if (OverlayWidgetController == nullptr)
    {
        OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
        OverlayWidgetController->SetWidgetControllerParams(WCParams);

        OverlayWidgetController->BindCallbacksToDependencies();
    }

    // ����Ѵ�����ֱ�ӷ�������ʵ��
    return OverlayWidgetController;
}

UAttributeMenuWidgetController* AAuraHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams)
{
    if (MenuWidgetController == nullptr)
    {
        MenuWidgetController = NewObject<UAttributeMenuWidgetController>(this,MenuWidgetControllerClass);
        MenuWidgetController->SetWidgetControllerParams(WCParams);
        MenuWidgetController->BindCallbacksToDependencies();
    }
    return MenuWidgetController;
}

/**
 * ��ʼ�����ǲ�UI����
 * 
 * @param PC ��ҿ�����ָ��
 * @param PS ���״ָ̬��  
 * @param ASC ����ϵͳ���ָ��
 * @param AS ���Լ�ָ��
 */
void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
    // ����Ҫ�����Ƿ������ã�ȷ��������ֿ�ָ���쳣
    checkf(OverlayWidgetClass, TEXT("Overlay Widget Class is nullptr"));
    checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class is nullptr"));

    // �������ǲ�ؼ�ʵ��
    UUserWidget* Widget = CreateWidget<UAuraUserWidget>(GetWorld(), OverlayWidgetClass);
    
    OverlayWidget = Cast<UAuraUserWidget>(Widget);

    // �����ؼ���������������ȡ�ؼ�������ʵ��
    const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
    UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);
    
    // ���ؼ��������󶨵�UI�ؼ���
    OverlayWidget->SetWidgetController(WidgetController);

    WidgetController->BroadcastInitialValues();

    // ���ؼ���ӵ��ӿ�����ʾ
    Widget->AddToViewport();

}