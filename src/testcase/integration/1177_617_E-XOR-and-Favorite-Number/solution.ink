// Translated from solution.cpp.

var S: dynamic = 316;

var N: dynamic = 2e5;

class d
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
  func t() -> dynamic
  {
      return (l / S);
    }
}

var q: dynamic = cpp_array(N);

func p(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a.t() != b.t())) ? (a.t() < b.t()) :  ((a.t() & 1)) ? (a.r > b.r) : (a.r < b.r);
}

var a: dynamic = cpp_array(N);

var c: dynamic = cpp_array((N * 12));

var r: dynamic = cpp_array(N);

var x: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var pl: dynamic = cpp_uninitialized();

var pr: dynamic = -1;

func add(v: dynamic) -> dynamic
{
  x += c[(v ^ k)];
  c[v] += 1;
}

func del(v: dynamic) -> dynamic
{
  c[v] -= 1;
  x -= c[(v ^ k)];
}

func main() -> dynamic
{
  read(n, m, k);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      a[i] ^= a[(i - 1)];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    while ((i < m))
    {
      read(q[i].l, q[i].r);
      q[i].id = i;
      i += 1;
    }
  }
  sort(q, (q + m), p);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var ql: dynamic = (q[i].l - 1);
      var qr: dynamic = q[i].r;
      var id: dynamic = q[i].id;
      while ((pl > ql))
      {
        add(a[cpp_update(pl, "--")]);
      }
      while ((pr < qr))
      {
        add(a[cpp_update(pr, "++")]);
      }
      while ((pl < ql))
      {
        del(a[cpp_update(pl, "++")]);
      }
      while ((pr > qr))
      {
        del(a[cpp_update(pr, "--")]);
      }
      r[id] = x;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      write(r[i], "\n");
      i += 1;
    }
  }
}
