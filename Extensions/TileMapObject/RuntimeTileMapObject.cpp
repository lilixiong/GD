/**

GDevelop - Tile Map Extension
Copyright (c) 2014-2015 Victor Levasseur (victorlevasseur52@gmail.com)
This project is released under the MIT License.
*/

#include "RuntimeTileMapObject.h"

#include <SFML/Graphics.hpp>
#include "GDCore/Tools/Localization.h"
#include "GDCpp/Object.h"
#include "GDCpp/Project.h"
#include "GDCpp/RuntimeScene.h"
#include "GDCpp/RuntimeGame.h"
#include "GDCpp/ImageManager.h"
#include "GDCpp/FontManager.h"
#include "GDCpp/Position.h"
#include "GDCpp/Polygon2d.h"
#include "GDCpp/PolygonCollision.h"
#include "GDCpp/ObjectsListsTools.h"
#include "GDCpp/Serialization/SerializerElement.h"
#include "GDCpp/CommonTools.h"

#include "TileMapObject.h"
#include "TileMap.h"
#include "TileSet.h"
#include "TileMapTools.h"

RuntimeTileMapObject::RuntimeTileMapObject(RuntimeScene & scene, const gd::Object & object) :
    RuntimeObject(scene, object),
    tileSet(),
    tileMap(),
    vertexArray(sf::Quads),
    oldX(0),
    oldY(0),
    needGeneration(false)
{
    const TileMapObject & tileMapObject = static_cast<const TileMapObject&>(object);

    tileSet = tileMapObject.tileSet;
    tileMap = tileMapObject.tileMap;

    //Load the tileset and generate the vertex array
    tileSet.Get().LoadResources(*(scene.game));
    tileSet.Get().Generate();
    vertexArray = TileMapExtension::GenerateVertexArray(tileSet.Get(), tileMap.Get());
    hitboxes = TileMapExtension::GenerateHitboxes(tileSet.Get(), tileMap.Get());
}

/**
 * Render object at runtime
 */
bool RuntimeTileMapObject::Draw( sf::RenderTarget& window )
{
    if(needGeneration)
    {
        //Re-generate the vertex array and the hitboxes
        vertexArray = TileMapExtension::GenerateVertexArray(tileSet.Get(), tileMap.Get());
        hitboxes = TileMapExtension::GenerateHitboxes(tileSet.Get(), tileMap.Get());
        for(std::vector<Polygon2d>::iterator it = hitboxes.begin(); it != hitboxes.end(); it++)
        {
            it->Move(GetX(), GetY());
        }
        needGeneration = false;
    }

    //Don't draw anything if hidden
    if ( hidden ) return true;

    //Get the current view
    sf::View currentView = window.getView();
    sf::Vector2f centerPos = currentView.getCenter();

    //Construct the transform
    sf::Transform transform;
    transform.translate((int)GetX() + centerPos.x - floor(centerPos.x),
                        (int)GetY() + centerPos.y - floor(centerPos.y));

    //Unsmooth the texture
    bool wasSmooth = tileSet.Get().GetTexture().isSmooth();
    tileSet.Get().GetTexture().setSmooth(false);

    //Draw the tilemap
    window.draw(vertexArray, sf::RenderStates(sf::BlendAlpha, transform, &tileSet.Get().GetTexture(), NULL));

    tileSet.Get().GetTexture().setSmooth(wasSmooth);

    return true;
}


float RuntimeTileMapObject::GetWidth() const
{
    if(tileSet.Get().IsDirty() || tileMap.Get().GetColumnsCount() == 0 || tileMap.Get().GetRowsCount() == 0)
        return 200.f;
    else
        return tileMap.Get().GetColumnsCount() * tileSet.Get().tileSize.x;
}

float RuntimeTileMapObject::GetHeight() const
{
    if(tileSet.Get().IsDirty() || tileMap.Get().GetColumnsCount() == 0 || tileMap.Get().GetRowsCount() == 0)
        return 150.f;
    else
        return tileMap.Get().GetRowsCount() * tileSet.Get().tileSize.y;
}

void RuntimeTileMapObject::OnPositionChanged()
{
    //Moves all hitboxes (use the previous pos to move them)
    for(std::vector<Polygon2d>::iterator it = hitboxes.begin(); it != hitboxes.end(); it++)
    {
        it->Move(GetX() - oldX, GetY() - oldY);
    }

    oldX = GetX();
    oldY = GetY();
}

#ifdef GD_IDE_ONLY
void RuntimeTileMapObject::GetPropertyForDebugger(unsigned int propertyNb, std::string & name, std::string & value) const
{

}

bool RuntimeTileMapObject::ChangeProperty(unsigned int propertyNb, std::string newValue)
{
    return true;
}

unsigned int RuntimeTileMapObject::GetNumberOfProperties() const
{
    return 0;
}
#endif

std::vector<Polygon2d> RuntimeTileMapObject::GetHitBoxes() const
{
    return hitboxes;
}

float RuntimeTileMapObject::GetTileWidth() const
{
    return tileSet.Get().tileSize.x;
}

float RuntimeTileMapObject::GetTileHeight() const
{
    return tileSet.Get().tileSize.y;
}

float RuntimeTileMapObject::GetMapWidth() const
{
    return static_cast<float>(tileMap.Get().GetColumnsCount());
}

float RuntimeTileMapObject::GetMapHeight() const
{
    return static_cast<float>(tileMap.Get().GetRowsCount());
}

float RuntimeTileMapObject::GetTile(int layer, int column, int row)
{
    if(layer < 0 || layer > 2 || column < 0 || column >= tileMap.Get().GetColumnsCount() || row < 0 || row >= tileMap.Get().GetRowsCount())
        return -1.f;

    return static_cast<float>(tileMap.Get().GetTile(layer, column, row));
}

void RuntimeTileMapObject::SetTile(int layer, int column, int row, int tileId)
{
    if(layer < 0 || layer > 2 || column < 0 || column >= tileMap.Get().GetColumnsCount() || row < 0 || row >= tileMap.Get().GetRowsCount())
        return;

    //Just update a single tile in the tile map
    tileMap.Get().SetTile(layer, column, row, tileId);
    TileMapExtension::UpdateVertexArray(vertexArray, layer, column, row, tileSet.Get(), tileMap.Get());
    TileMapExtension::UpdateHitboxes(hitboxes, sf::Vector2f(GetX(), GetY()), layer, column, row, tileSet.Get(), tileMap.Get());
}

float RuntimeTileMapObject::GetColumnAt(float x)
{
    return static_cast<float>(floor((x - GetX())/tileSet.Get().tileSize.x));
}

float RuntimeTileMapObject::GetRowAt(float y)
{
    return static_cast<float>(floor((y - GetY())/tileSet.Get().tileSize.y));
}

std::string RuntimeTileMapObject::SaveAsString() const
{
    return tileMap.Get().SerializeToString();
}

void RuntimeTileMapObject::LoadFromString(const std::string &str)
{
    tileMap.Get().UnserializeFromString(str);
    needGeneration = true;
}

void RuntimeTileMapObject::ChangeTexture(const std::string &textureName, RuntimeScene &scene)
{
    tileSet.Get().textureName = textureName;
    tileSet.Get().LoadResources(*(scene.game));
    tileSet.Get().Generate();
    needGeneration = true;
}

bool GD_EXTENSION_API SingleTileCollision(std::map<std::string, std::vector<RuntimeObject*>*> tileMapList,
                         int layer,
                         int column,
                         int row,
                         std::map<std::string, std::vector<RuntimeObject*>*> objectLists,
                         bool conditionInverted)
{
    return TwoObjectListsTest(tileMapList, objectLists, conditionInverted, [layer, column, row](RuntimeObject* tileMapObject_, RuntimeObject * object) {
        RuntimeTileMapObject *tileMapObject = dynamic_cast<RuntimeTileMapObject*>(tileMapObject_);
        if(!tileMapObject || tileMapObject->tileSet.Get().IsDirty())
            return false;

        //Get the tile hitbox
        int tileId = tileMapObject->tileMap.Get().GetTile(layer, column, row);
        if(tileId < 0 || tileId >= tileMapObject->tileSet.Get().GetTilesCount())
            return false;

        Polygon2d tileHitbox = tileMapObject->tileSet.Get().GetTileHitbox(tileId).hitbox;
        tileHitbox.Move(tileMapObject->GetX() + column * tileMapObject->tileSet.Get().tileSize.x,
                        tileMapObject->GetY() + row * tileMapObject->tileSet.Get().tileSize.y);

        //Get the object hitbox
        std::vector<Polygon2d> objectHitboxes = object->GetHitBoxes();

        for(std::vector<Polygon2d>::iterator hitboxIt = objectHitboxes.begin(); hitboxIt != objectHitboxes.end(); ++hitboxIt)
        {
            if(PolygonCollisionTest(tileHitbox, *hitboxIt).collision)
            {
                return true;
            }
        }

        return false;
    });
}

RuntimeObject * CreateRuntimeTileMapObject(RuntimeScene & scene, const gd::Object & object)
{
    return new RuntimeTileMapObject(scene, object);
}
