#include "camera.h"

#include <cmath>

void Camera::setup(Point from, Point at, Vector up, floating fov,
                   floating hither, size_t width, size_t height) {
    const auto near = hither;
    const auto far = 10000.0;
    const auto asp = (floating)width / height;

    const auto MV = Matrix::lookAt(from, at, up);
    const auto P = Matrix::perspective(fov, asp, near, far);
    const auto MVP = P * MV;
    const auto invMVP = invert(MVP);

    m_eye = from;
    m_width = width;
    m_height = height;
    m_forward = normalised(at - from);
    m_toScreen = Matrix::scale(width / 2.0, height / 2.0, 0) *
                 Matrix::translate(1.0, 1.0, 0.0) * MVP;
    m_fromScreen = invMVP * Matrix::translate(-1.0, -1.0, 0.0) *
                   Matrix::scale(2.0 / width, 2.0 / height, 0.0);
    m_imagePlaneDistance = width / (2.0 * tan(fov * M_PI / 360.0));
}

bool Camera::raster(Point worldPoint, floating& x, floating& y) const {
    const auto eyeToWorldPoint = worldPoint - m_eye;
    const auto screenPoint = m_toScreen.transform(worldPoint);
    x = screenPoint.x;
    y = screenPoint.y;
    return eyeToWorldPoint.dot(m_forward) > 0.0 && 0 <= screenPoint.x &&
           screenPoint.x < m_width && 0 <= screenPoint.y &&
           screenPoint.y < m_height;
}

Ray Camera::spawnRay(floating x, floating y) const {
    const auto screenPoint = Point{x, y, 0.0};
    const auto worldPoint = m_fromScreen.transform(screenPoint);
    const auto direction = normalised(worldPoint - m_eye);
    return {m_eye, direction};
}
