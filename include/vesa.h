#ifndef VESA_H
#define VESA_H

#include "types.h"

typedef struct kVBEInfoBlockStruct
{
    //==========================================================================
    // 모든 VBE 버전에 공통인 부분
    //==========================================================================
    uint16_t wModeAttribute;        // 모드의 속성
    uint8_t bWinAAttribute;        // 윈도우 A의 속성
    uint8_t bWinBAttribute;        // 윈도우 B의 속성
    uint16_t wWinGranulity;         // 윈도우의 가중치(Granularity)
    uint16_t wWinSize;              // 윈도우의 크기
    uint16_t wWinASegment;          // 윈도우 A가 시작하는 세그먼트 어드레스
    uint16_t wWinBSegment;          // 윈도우 B가 시작하는 세그먼트 어드레스
    uint32_t dwWinFuncPtr;         // 윈도우 관련 함수의 포인터(리얼 모드 용)
    uint16_t wuint8_tsPerScanLine;     // 화면 스캔 라인(Scan Line) 당 바이트 수
    
    //==========================================================================
    // VBE 버전 1.2 이상 공통인 부분
    //==========================================================================
    uint16_t wXResolution;          // X축 픽셀 수 또는 문자 수
    uint16_t wYResolution;          // Y축 픽셀 수 또는 문자 수
    uint8_t bXCharSize;            // 한 문자의 X축 픽셀 수
    uint8_t bYCharSize;            // 한 문자의 Y축 픽셀 수
    uint8_t bNumberOfPlane;        // 메모리 플레인(Memory Plane) 수
    uint8_t bBitsPerPixel;         // 한 픽셀을 구성하는 비트 수
    uint8_t bNumberOfBanks;        // 뱅크(Bank) 수
    uint8_t bMemoryModel;          // 비디오 메모리 구성
    uint8_t bBankSize;             // 뱅크의 크기(Kuint8_t)
    uint8_t bNumberOfImagePages;   // 이미지 페이지 개수
    uint8_t bReserved;             // 페이지 기능을 위해 예약된 영역
    
    // 다이렉트 컬러(Direct Color)에 관련된 필드
    uint8_t bRedMaskSize;              // 빨간색(Red) 필드가 차지하는 크기
    uint8_t bRedFieldPosition;         // 빨간색 필드의 위치
    uint8_t bGreenMaskSize;            // 녹색(Green) 필드가 차지하는 크기
    uint8_t bGreenFieldPosition;       // 녹색 필드의 위치
    uint8_t bBlueMaskSize;             // 파란색(Blue) 필드가 차지하는 크기
    uint8_t bBlueFieldPosition;        // 파란색 필드의 위치
    uint8_t bReservedMaskSize;         // 예약된 필드의 크기
    uint8_t bReservedFieldPosition;    // 예약된 필드의 위치
    uint8_t bDirectColorModeInfo;      // 다이렉트 컬러 모드의 정보
    
    //==========================================================================
    // VBE 버전 2.0 이상 공통인 부분
    //==========================================================================
    uint32_t dwPhysicalBasePointer;    // 선형 프레임 버퍼 메모리의 시작 어드레스
    uint32_t dwReserved1;              // 예약된 필드
    uint32_t dwReserved2;
    
    //==========================================================================
    // VBE 버전 3.0 이상 공통인 부분
    //==========================================================================
    uint16_t wLinearuint8_tsPerScanLine;       // 선형 프레임 버퍼 모드의 
                                        // 화면 스캔 라인(Scan Line) 당 바이트 수
    uint8_t bBankNumberOfImagePages;       // 뱅크 모드일 때 이미지 페이지 수
    uint8_t bLinearNumberOfImagePages;     // 선형 프레임 버퍼 모드일 때 이미지 페이지 수
    // 선형 프레임 버퍼 모드일 때 다이렉트 컬러(Direct Color)에 관련된 필드
    uint8_t bLinearRedMaskSize;            // 빨간색(Red) 필드가 차지하는 크기 
    uint8_t bLinearRedFieldPosition;       // 빨간색 필드의 위치
    uint8_t bLinearGreenMaskSize;          // 녹색(Green) 필드가 차지하는 크기
    uint8_t bLinearGreenFieldPosition;     // 녹색 필드의 위치
    uint8_t bLinearBlueMaskSize;           // 파란색(Blue) 필드가 차지하는 크기
    uint8_t bLinearBlueFieldPosition;      // 파란색 필드의 위치
    uint8_t bLinearReservedMaskSize;       // 예약된 필드의 크기
    uint8_t bLinearReservedFieldPosition;  // 예약된 필드의 위치
    uint32_t dwMaxPixelClock;              // 픽셀 클록의 최대 값(Hz)

    uint8_t vbReserved[ 189 ];             // 나머지 영역
} VBEMODEINFOBLOCK;

#define modeInfo ((VBEMODEINFOBLOCK *)0x9000)

#endif