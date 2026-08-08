// Translated from solution.cpp.

var S = 316;

var N = 2e5;

class d
{
  var l: dynamic;
  var r: dynamic;
  var id: dynamic;
  func t()
  {
      return (l / S);
    }
}

var q = cpp_array(N);

func p(a: dynamic, b: dynamic)
{
  return if ((a.t() != b.t())) (a.t() < b.t()) else if ((a.t() & 1)) (a.r > b.r) else (a.r < b.r);
}

var a = cpp_array(N);

var c = cpp_array((N * 12));

var r = cpp_array(N);

var x: dynamic;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var pl: dynamic;

var pr = -1;

func add(v: dynamic)
{
  x += c[(v ^ k)];
  c[v] += 1;
}

func del(v: dynamic)
{
  c[v] -= 1;
  x -= c[(v ^ k)];
}

func main()
{
  read(n, m, k);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      a[i] ^= a[(i - 1)];
      i += 1;
    }
  }
  {
    var i = 0;
    var l: dynamic;
    var r: dynamic;
    while ((i < m))
    {
      read(q[i].l, q[i].r);
      q[i].id = i;
      i += 1;
    }
  }
  sort(q, (q + m), p);
  {
    var i = 0;
    while ((i < m))
    {
      var ql = (q[i].l - 1);
      var qr = q[i].r;
      var id = q[i].id;
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
    var i = 0;
    while ((i < m))
    {
      write(r[i], "\n");
      i += 1;
    }
  }
}
