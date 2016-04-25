Name:           libbus1
Version:        1
Release:        1
Summary:        Bus1 IPC Library
License:        LGPL2+
URL:            https://github.com/bus1/libbus1
Source0:        %{name}.tar.xz
BuildRequires:  autoconf automake pkgconfig
BuildRequires:  crbtree-devel
BuildRequires:  cvariant-devel

%description
bus1 IPC Library

%package        devel
Summary:        Development files for %{name}
Requires:       %{name} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%prep
%setup -q

%build
./autogen.sh
%configure
make %{?_smp_mflags}

%install
%make_install

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%doc COPYING
%{_bindir}/org.bus1.busctl
%{_libdir}/libbus1.so.*

%files devel
%{_includedir}/org.bus1/*.h
%{_libdir}/libbus1.so
%{_libdir}/pkgconfig/libbus1.pc

%changelog
* Sun Apr 24 2016 <kay@redhat.com> 1-1
- libbus1 1
