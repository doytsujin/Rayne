//
//  RNParticleEmitter.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_PARTICLEEMITTER_H__
#define __RAYNE_PARTICLEEMITTER_H__

#include "RNBase.h"
#include "RNSceneNode.h"
#include "RNParticle.h"
#include "RNMaterial.h"
#include "RNMesh.h"
#include "RNTexture.h"
#include "RNRandom.h"

namespace RN
{
	class ParticleEmitter;
	class ParticleMaterial : public Material
	{
	public:
		ParticleMaterial();
		~ParticleMaterial() override;
		
		Vector3 minVelocity;
		Vector3 maxVelocity;
		
		float lifespan;
		float lifespanVariance;
		
		RNDefineMetaWithTraits(ParticleMaterial, Material, MetaClassTraitCronstructable)
	};
	
	class ParticleEmitter : public SceneNode
	{
	public:
		ParticleEmitter();
		~ParticleEmitter() override;
		
		void Cook(float time, int steps);
		void SetMaterial(ParticleMaterial *material);
		void SetGenerator(RandomNumberGenerator *generator);
		
		void SetSpawnRate(float spawnRate);
		void SetParticlesPerSecond(size_t particles);
		void SetMaxParticles(size_t maxParticles);
		
		RandomNumberGenerator *GetGenerator() { return _rng; }
		
		void SpawnParticles(size_t particles);
		Particle *SpawnParticle();
		
		void Update(float delta) override;
		bool IsVisibleInCamera(Camera *camera) override;
		void Render(Renderer *renderer, Camera *camera) override;
		
	protected:
		virtual Particle *CreateParticle();
		RandomNumberGenerator *_rng;
		
	private:
		void UpdateParticles(float delta);
		void UpdateMesh();
		
		std::vector<Particle *> _particles;
		size_t _maxParticles;
		
		ParticleMaterial *_material;
		Mesh *_mesh;
		
		float _spawnRate;
		float _accDelta;
		
		RNDefineMetaWithTraits(ParticleEmitter, SceneNode, MetaClassTraitCronstructable)
	};
}

#endif /* __RAYNE_PARTICLEEMITTER_H__ */
