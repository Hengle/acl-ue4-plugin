#pragma once

// Copyright 2018 Nicholas Frechette. All Rights Reserved.

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimBoneCompressionCodec.h"

#include "ACLImpl.h"

#if WITH_EDITORONLY_DATA
#include <acl/compression/compression_settings.h>
#include <acl/compression/track_array.h>
#include <acl/core/compressed_database.h>
#include <acl/core/iallocator.h>
#endif

#include <acl/core/compressed_tracks.h>

#include "AnimBoneCompressionCodec_ACLBase.generated.h"

/** An enum that represents the result of attempting to use a safety fallback codec. */
enum class ACLSafetyFallbackResult
{
	Success,	// Safety fallback is used and compressed fine
	Failure,	// Safety fallback is used but failed to compress
	Ignored,	// No safety fallback used
};

struct FACLCompressedAnimData final : public ICompressedAnimData
{
	/** Holds the compressed_tracks instance */
	TArrayView<uint8> CompressedByteStream;

	const acl::compressed_tracks* GetCompressedTracks() const { return acl::make_compressed_tracks(CompressedByteStream.GetData()); }

	// ICompressedAnimData implementation
	virtual void Bind(const TArrayView<uint8> BulkData) override { CompressedByteStream = BulkData; }
	virtual int64 GetApproxCompressedSize() const override { return CompressedByteStream.Num(); }
	virtual bool IsValid() const override;
};

/** The base codec implementation for ACL support. */
UCLASS(abstract, MinimalAPI)
class UAnimBoneCompressionCodec_ACLBase : public UAnimBoneCompressionCodec
{
	GENERATED_UCLASS_BODY()

#if WITH_EDITORONLY_DATA
	/** The compression level to use. Higher levels will be slower to compress but yield a lower memory footprint. */
	UPROPERTY(EditAnywhere, Category = "ACL Options")
	TEnumAsByte<ACLCompressionLevel> CompressionLevel;

	/** How to treat phantom tracks. Phantom tracks are not mapped to a skeleton bone. */
	UPROPERTY(EditAnywhere, Category = "ACL Options")
	ACLPhantomTrackMode PhantomTrackMode;

	/** The default virtual vertex distance for normal bones. */
	UPROPERTY(EditAnywhere, Category = "ACL Options", meta = (ClampMin = "0"))
	float DefaultVirtualVertexDistance;

	/** The virtual vertex distance for bones that requires extra accuracy. */
	UPROPERTY(EditAnywhere, Category = "ACL Options", meta = (ClampMin = "0"))
	float SafeVirtualVertexDistance;

	/** The error threshold to use when optimizing and compressing the animation sequence. */
	UPROPERTY(EditAnywhere, Category = "ACL Options", meta = (ClampMin = "0"))
	float ErrorThreshold;
#endif

	/** Whether or not to strip the bind pose from compressed clips. UE 5.1+ */
	UPROPERTY(EditAnywhere, Category = "ACL Options", meta = (EditCondition = "bIsBindPoseStrippingSupported", HideEditConditionToggle))
	bool bStripBindPose;

#if WITH_EDITORONLY_DATA
	/** Whether bind pose stripping is supported or not. Only used in the editor to enable/disable the feature. */
	UPROPERTY(Transient)
	bool bIsBindPoseStrippingSupported;
#endif

#if WITH_EDITORONLY_DATA
	// UAnimBoneCompressionCodec implementation
	virtual bool Compress(const FCompressibleAnimData& CompressibleAnimData, FCompressibleAnimDataResult& OutResult) override;

#if (ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1)
	virtual void PopulateDDCKey(const UE::Anim::Compression::FAnimDDCKeyArgs& KeyArgs, FArchive& Ar) override;
#else
	virtual void PopulateDDCKey(FArchive& Ar) override;
#endif

	// Our implementation
	virtual void PostCompression(const FCompressibleAnimData& CompressibleAnimData, FCompressibleAnimDataResult& OutResult) const {}
	virtual void GetCompressionSettings(acl::compression_settings& OutSettings) const PURE_VIRTUAL(UAnimBoneCompressionCodec_ACLBase::GetCompressionSettings, );
	virtual TArray<class USkeletalMesh*> GetOptimizationTargets() const { return TArray<class USkeletalMesh*>(); }
	virtual ACLSafetyFallbackResult ExecuteSafetyFallback(acl::iallocator& Allocator, const acl::compression_settings& Settings, const acl::track_array_qvvf& RawClip, const acl::track_array_qvvf& BaseClip, const acl::compressed_tracks& CompressedClipData, const FCompressibleAnimData& CompressibleAnimData, FCompressibleAnimDataResult& OutResult);
#endif

	// UAnimBoneCompressionCodec implementation
	virtual TUniquePtr<ICompressedAnimData> AllocateAnimData() const override;
	virtual void ByteSwapIn(ICompressedAnimData& AnimData, TArrayView<uint8> CompressedData, FMemoryReader& MemoryStream) const override;
	virtual void ByteSwapOut(ICompressedAnimData& AnimData, TArrayView<uint8> CompressedData, FMemoryWriter& MemoryStream) const override;
};
