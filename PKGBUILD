pkgname="mcabber-module-focus"
pkgver=1.e757efa
pkgrel=1
pkgdesc="autoleaving focus for mcabber"
url="https://github.com/seletskiy/mcabber-focus"
arch=('any')
license=('GPL')
makedepends=()
source=(git+https://github.com/seletskiy/mcabber-focus)
md5sums=(SKIP)

pkgver() {
    cd "mcabber-focus"
    echo $(git rev-list --count master).$(git rev-parse --short master)
}

build() {
    cd "$srcdir/mcabber-focus"

    make
}

package() {
    install -D "$srcdir/mcabber-focus/libfocus.so" "$pkgdir/usr/lib/mcabber/libfocus.so"
}
