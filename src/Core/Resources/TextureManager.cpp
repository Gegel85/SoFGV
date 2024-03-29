//
// Created by PinkySmile on 18/09/2021
//

#include "TextureManager.hpp"
#include "Game.hpp"
#include "../Logger.hpp"

namespace SpiralOfFate
{
	unsigned TextureManager::load(std::string file, Vector2u *size)
	{
		for (auto pos = file.find('\\'); pos != std::string::npos; pos = file.find('\\'))
			file[pos] = '/';
		if (this->_allocatedTextures[file].second != 0) {
			this->_allocatedTextures[file].second++;
			game->logger.debug("Returning already loaded file " + file);
			if (size)
				*size = this->_textures[this->_allocatedTextures[file].first].getSize();
			return this->_allocatedTextures[file].first;
		}

		unsigned index;

		if (this->_freedIndexes.empty()) {
			this->_lastIndex += this->_lastIndex == 0;
			index = this->_lastIndex;
			this->_lastIndex++;
		} else {
			index = this->_freedIndexes.back();
			this->_freedIndexes.pop_back();
		}

		game->logger.debug("Loading file " + file);
		if (!this->_textures[index].loadFromFile(file)) {
			this->_freedIndexes.push_back(index);
			return 0;
		}

		if (size)
			*size = this->_textures[index].getSize();

		this->_allocatedTextures[file].first = index;
		this->_allocatedTextures[file].second = 1;
		return index;
	}

	unsigned TextureManager::load(const std::string &file, std::pair<std::vector<Color>, std::vector<Color>> palette, Vector2u *size)
	{
		std::string allocName = file;
		bool ok = false;
		Vector2u realSize;

		my_assert_eq(palette.first.size(), palette.second.size());
		if (palette.first.empty())
			return this->load(file, size);
		for (auto pos = allocName.find('\\'); pos != std::string::npos; pos = allocName.find('\\'))
			allocName[pos] = '/';
		allocName += ":paletted";
		for (unsigned i = 0; i < palette.first.size(); i++) {
			palette.first[i].a = palette.second[i].a = 255;
			if (palette.first[i] != palette.second[i]) {
				allocName += "|" + std::to_string(palette.first[i].value) + "," + std::to_string(palette.second[i].value);
				ok = true;
			}
		}
		if (this->_allocatedTextures[allocName].second != 0) {
			this->_allocatedTextures[allocName].second++;
			game->logger.debug("Returning already loaded paletted file " + allocName);
			if (size)
				*size = this->_textures[this->_allocatedTextures[allocName].first].getSize();
			return this->_allocatedTextures[allocName].first;
		}
		if (!ok)
			return this->load(file, size);

		auto pixels = TextureManager::loadPixels(file, realSize);

		for (unsigned x = 0; x < realSize.x; x++)
			for (unsigned y = 0; y < realSize.y; y++) {
				auto &color = pixels[x + y * realSize.x];
				auto it = std::find_if(palette.first.begin(), palette.first.end(), [color](Color &a){
					return a.r == color.r && a.g == color.g && a.b == color.b;
				});

				if (it != palette.first.end())
					color = palette.second[it - palette.first.begin()];
			}

		unsigned index = this->load(pixels, realSize);

		delete[] pixels;
		if (index) {
			if (size)
				*size = realSize;

			this->_allocatedTextures[allocName].first = index;
			this->_allocatedTextures[allocName].second = 1;
		}
		return index;
	}

	unsigned TextureManager::load(const Color *pixels, Vector2u size)
	{
		sf::Image image;

		image.create(size.x, size.y, reinterpret_cast<const sf::Uint8 *>(pixels));
		if (!this->_textures[this->_lastIndex].loadFromImage(image))
			return 0;
		return this->_lastIndex++;
	}

	Color *TextureManager::loadPixels(const std::string &file, Vector2u &size)
	{
		sf::Image image;
		Color *buffer;

		if (!image.loadFromFile(file))
			return nullptr;
		size = image.getSize();
		buffer = new Color[size.x * size.y];

		auto ptr = image.getPixelsPtr();

		for (unsigned x = 0; x < size.x; x++)
			for (unsigned y = 0; y < size.y; y++) {
				buffer[x + y * size.y].r = reinterpret_cast<const sf::Color *>(ptr)[x + y * size.y].r;
				buffer[x + y * size.y].g = reinterpret_cast<const sf::Color *>(ptr)[x + y * size.y].g;
				buffer[x + y * size.y].b = reinterpret_cast<const sf::Color *>(ptr)[x + y * size.y].b;
				buffer[x + y * size.y].a = reinterpret_cast<const sf::Color *>(ptr)[x + y * size.y].a;
			}
		return buffer;
	}

	void TextureManager::remove(unsigned int id)
	{
		if (!id)
			return;

		for (auto &[loadedPath, attr] : this->_allocatedTextures)
			if (attr.first == id && attr.second) {
				attr.second--;
				if (attr.second) {
					game->logger.debug("Remove ref to " + loadedPath);
					return;
				}
				game->logger.debug("Destroying texture " + loadedPath);
				break;
			}

		auto it = this->_textures.find(id);

		my_assert(it != this->_textures.end());
		this->_textures.erase(it);
		this->_freedIndexes.push_back(id);
	}

	void TextureManager::render(Sprite &sprite) const
	{
		if (!sprite.textureHandle)
			return;
		sprite.setTexture(this->_textures.at(sprite.textureHandle));
		game->screen->displayElement(sprite);
	}

	void TextureManager::addRef(unsigned int id)
	{
		if (id == 0)
			return;
		for (auto &[loadedPath, attr] : this->_allocatedTextures)
			if (attr.first == id && attr.second) {
				attr.second++;
				my_assert(attr.second > 1);
				game->logger.debug("Adding ref to " + loadedPath);
				return;
			}
	}

	Vector2u TextureManager::getTextureSize(unsigned int id) const
	{
		if (id == 0)
			return {0, 0};
		return this->_textures.at(id).getSize();
	}
}
