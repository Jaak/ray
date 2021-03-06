#pragma once

#include <string>

class Scene;

class SceneReader {
public: /* Types: */
    enum Status { OK, E_PARSE, E_MEM, E_OTHER };

public: /* Methods: */
    SceneReader(char const* fname)
        : m_fname(fname) {}

    virtual ~SceneReader() {}

    virtual Status init(Scene& scene) const = 0;

    std::string getFname() { return m_fname; }

protected:
    std::string const& fname() const { return m_fname; }

private: /* Fields: */
    std::string m_fname;
};
