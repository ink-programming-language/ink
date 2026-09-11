// Translated from solution.cpp.

var Maxn: dynamic = 3005;

var Maxm: dynamic = 300005;

var Mo: dynamic = 1000000007;

var sp: dynamic = [[0, 1], [0, -1], [1, 0], [-1, 0], [1, 1], [-1, -1], [1, -1], [-1, 1]];

var oo: dynamic = (INT_MAX >> 2);

class Tree
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var cnt: dynamic = cpp_uninitialized();
}

class edge
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var len: dynamic = cpp_uninitialized();
}

class point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var f: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var mp: dynamic = cpp_array((Maxn * 2), Maxn);

func gf(w: dynamic) -> dynamic
{
  if ((f[w] == w))
  {
    return w;
  }
  return cpp_assign(f[w], "=", gf(f[w]));
}

func in_cpp(x: dynamic, y: dynamic) -> dynamic
{
  return (((((x >= 1) && (x <= n)) && (y >= 1)) && (y <= (m * 2))));
}

func fd(p: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  p.clear();
  {
    var i: dynamic = 0;
    while ((i < 8))
    {
      var cx: dynamic = (x + sp[i][0]);
      var cy: dynamic = (y + sp[i][1]);
      if ((cy < 1))
      {
        cy = (2 * m);
      }
      if ((cy > (2 * m)))
      {
        cy = 1;
      }
      if (((!in_cpp(cx, cy)) || (!mp[cx][cy])))
      {
        i += 1;
        continue;
      }
      var a: dynamic = gf(mp[cx][cy]);
      p.push_back((a));
      i += 1;
    }
  }
}

func ck(x: dynamic, y: dynamic) -> dynamic
{
  var p1: dynamic = cpp_uninitialized();
  var p2: dynamic = cpp_uninitialized();
  fd(p1, x, y);
  fd(p2, x, (y + m));
  {
    var i: dynamic = 0;
    while ((i < p1.size()))
    {
      {
        var j: dynamic = 0;
        while ((j < p2.size()))
        {
          if ((p1[i] == p2[j]))
          {
            return 0;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return 1;
}

func alt(id: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  mp[x][y] = id;
  {
    var i: dynamic = 0;
    while ((i < 8))
    {
      var cx: dynamic = (x + sp[i][0]);
      var cy: dynamic = (y + sp[i][1]);
      if ((cy < 1))
      {
        cy = (2 * m);
      }
      if ((cy > (2 * m)))
      {
        cy = 1;
      }
      if (((!in_cpp(cx, cy)) || (!mp[cx][cy])))
      {
        i += 1;
        continue;
      }
      var a: dynamic = gf(mp[cx][cy]);
      f[a] = id;
      i += 1;
    }
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  read(n, m, c);
  if ((m == 1))
  {
    write(0, "\n");
    return 0;
  }
  {
    i = 1;
    while ((i <= c))
    {
      read(x, y);
      if (mp[x][y])
      {
        ans += 1;
        i += 1;
        continue;
      }
      f[i] = i;
      f[(i + c)] = (i + c);
      if (ck(x, y))
      {
        ans += 1;
        alt(i, x, y);
        alt((i + c), x, (y + m));
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
