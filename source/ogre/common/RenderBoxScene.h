/*!
	@file
	@author		Evmenov Georgiy
	@date		01/2008
	@module
*/

#pragma once
#include <Ogre.h>
#include <MyGUI.h>
#include "RenderBox.h"

namespace wraps
{

	class RenderBoxScene : public RenderBox /*Canvas*/
	{
	public:
		RenderBoxScene() :
			mScene(nullptr),
			mNode(nullptr),
			mCameraNode(nullptr),
			mCamera(nullptr),
			mEntity(nullptr),
			mAnimationState(nullptr),
			mRotationSpeed(RENDER_BOX_AUTO_ROTATION_SPEED),
			mMouseRotation(false),
			mLastPointerX(0),
			mMousePressed(false),
			mAutoRotation(false),
			mFrameAdvise(false)
		{
		}

		virtual ~RenderBoxScene()
		{
		}

		/** default RenderBox autorotation speed (if enabled) */
		enum { RENDER_BOX_AUTO_ROTATION_SPEED = 20 };


		/** Add mesh to scene and remove previous one
			@param
				_meshName The name of the Mesh it is to be based on (e.g. 'ogrehead.mesh').
		*/
		void injectObject(const Ogre::String& _meshName, const Ogre::Vector3& _position = Ogre::Vector3::ZERO, const Ogre::Quaternion& _orientation = Ogre::Quaternion::IDENTITY, const Ogre::Vector3& _scale = Ogre::Vector3::UNIT_SCALE)
		{
			clearScene();

			static size_t num = 0;
			mEntity = mScene->createEntity(MyGUI::utility::toString(this, "_RenderBoxMesh_", _meshName, num++), _meshName);
			Ogre::SceneNode* node = mNode->createChildSceneNode(_position, _orientation);
			node->attachObject(mEntity);

			updateViewport();
		}

		/** Run mesh animation if animation with such name exist (else print warning in log).
			To stop animation use empty string.
		*/
		void setAnimation(const Ogre::String& _value)
		{
			if (mEntity == nullptr)
				return;

			try
			{
				mAnimationState = mEntity->getAnimationState(_value);
				mAnimationState->setEnabled(true);
				mAnimationState->setLoop(true);
				mAnimationState->setWeight(1);
			}
			catch (Ogre::ItemIdentityException& _e)
			{
				MYGUI_LOG(Warning, _e.getDescription());
			}

			frameAdvise(needFrameUpdate());
		}

		/** Clear scene */
		void clearScene()
		{
			setRotationAngle(Ogre::Degree(0));

			if (mScene)
			{
				mScene->destroyAllEntities();
				mNode->removeAndDestroyAllChildren();
			}

			mEntity = nullptr;
			mAnimationState = nullptr;
		}

		/** Set speed of entity rotation.
			@param
				_speed of rotation in degrees per second.
		*/
		void setAutoRotationSpeed(int _value = RENDER_BOX_AUTO_ROTATION_SPEED)
		{
			mRotationSpeed = _value;
		}

		/** Get speed of entity rotation.*/
		int getAutoRotationSpeed()
		{
			return mRotationSpeed;
		}

		/** Enable or disable auto rotation */
		void setAutoRotation(bool _value)
		{
			mAutoRotation = _value;
			frameAdvise(needFrameUpdate());
		}

		/** Get auto rotation flag */
		bool getAutoRotation()
		{
			return mAutoRotation;
		}

		/** Set start rotation angle of entity. */
		void setRotationAngle(const Ogre::Degree& _value)
		{
			if (mNode)
			{
				mNode->resetOrientation();
				mNode->yaw(Ogre::Radian(_value));
			}
		}

		/** Get rotation angle of entity. */
		Ogre::Degree getRotationAngle()
		{
			return Ogre::Degree(mNode->getOrientation().getYaw());
		}

		/** Set possibility to rotate mesh by mouse drag. */
		void setMouseRotation(bool _value)
		{
			mMouseRotation = _value;
		}

		/** Get mouse rotation flag */
		bool getMouseRotation()
		{
			return mMouseRotation;
		}

		virtual void setCanvas(MyGUI::Canvas* _value)
		{
			RenderBox::setCanvas(_value);

			mCanvas->eventMouseDrag += newDelegate(this, &RenderBoxScene::notifyMouseDrag);
			mCanvas->eventMouseButtonPressed += newDelegate(this, &RenderBoxScene::notifyMouseButtonPressed);
			mCanvas->eventMouseButtonReleased += newDelegate(this, &RenderBoxScene::notifyMouseButtonReleased);

			createScene();
		}

		virtual void destroy()
		{
			clearScene();

			if (mCanvas)
			{
				frameAdvise(false);

				mCanvas->eventMouseDrag -= newDelegate(this, &RenderBoxScene::notifyMouseDrag);
				mCanvas->eventMouseButtonPressed -= newDelegate(this, &RenderBoxScene::notifyMouseButtonPressed);
				mCanvas->eventMouseButtonReleased -= newDelegate(this, &RenderBoxScene::notifyMouseButtonReleased);
				mCanvas = nullptr;

				Ogre::Root* root = Ogre::Root::getSingletonPtr();
				if (root && mScene)
					root->destroySceneManager(mScene);
				mScene = nullptr;
				mNode = nullptr;
			}

			RenderBox::destroy();
		}

	private:
		void notifyMouseDrag(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
		{
			if (mMouseRotation)
			{
				if (mNode)
					mNode->yaw(Ogre::Radian(Ogre::Degree((float)_left - mLastPointerX)));
				mLastPointerX = _left;
			}
		}

		void notifyMouseButtonPressed(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
		{
			if (mMouseRotation)
			{
				if (_id == MyGUI::MouseButton::Left)
				{
					const MyGUI::IntPoint& point = MyGUI::InputManager::getInstance().getLastPressedPosition(MyGUI::MouseButton::Left);
					mLastPointerX = point.left;
					mMousePressed = true;
				}
				if (_id == MyGUI::MouseButton::Right)
				{
					const MyGUI::IntPoint& point = MyGUI::InputManager::getInstance().getLastPressedPosition(MyGUI::MouseButton::Right);
					mLastPointerX = point.left;
					mMousePressed = true;
				}
			}
		}

		void notifyMouseButtonReleased(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
		{
			mMousePressed = false;
		}

		void frameAdvise(bool _advise)
		{
			if (_advise && !mFrameAdvise && needFrameUpdate())
			{
				MyGUI::Gui::getInstance().eventFrameStart += MyGUI::newDelegate(this, &RenderBoxScene::frameEntered);
				mFrameAdvise = true;
			}
			else if (!_advise && mFrameAdvise && !needFrameUpdate())
			{
				MyGUI::Gui::getInstance().eventFrameStart -= MyGUI::newDelegate(this, &RenderBoxScene::frameEntered);
				mFrameAdvise = false;
			}
		}

		void frameEntered(float _time)
		{
			if (!mMousePressed)
			{
				if (mAutoRotation && mNode)
					mNode->yaw(Ogre::Radian(Ogre::Degree(_time * mRotationSpeed)));
			}
			if (mAnimationState)
				mAnimationState->addTime(_time);
		}

		bool needFrameUpdate()
		{
			return mAutoRotation || mAnimationState;
		}


		void createScene();

		void updateViewport();
		

		virtual void requestUpdateCanvas(MyGUI::Canvas* _canvas, MyGUI::Canvas::Event _event)
		{
			RenderBox::requestUpdateCanvas(_canvas, _event);

			updateViewport();
		}

	private:
		Ogre::SceneManager* mScene;
		Ogre::SceneNode* mNode;
		Ogre::SceneNode* mCameraNode;
		Ogre::Camera* mCamera;
		Ogre::Entity* mEntity;
		Ogre::AnimationState* mAnimationState;

		int mRotationSpeed;
		bool mMouseRotation;
		int mLastPointerX;
		bool mMousePressed;
		bool mAutoRotation;

		bool mFrameAdvise;
	};

} // namespace wraps
