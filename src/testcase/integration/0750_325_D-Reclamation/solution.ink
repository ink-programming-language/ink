// Translated from solution.cpp.

var Maxn = 3005;

var Maxm = 300005;

var Mo = 1000000007;

var sp = [[0, 1], [0, -1], [1, 0], [-1, 0], [1, 1], [-1, -1], [1, -1], [-1, 1]];

var oo = (INT_MAX >> 2);

class Tree
{
  var l: dynamic;
  var r: dynamic;
  var cnt: dynamic;
}

class edge
{
  var u: dynamic;
  var v: dynamic;
  var len: dynamic;
}

class point
{
  var x: dynamic;
  var y: dynamic;
  var id: dynamic;
}

var f: dynamic;

var n: dynamic;

var m: dynamic;

var c: dynamic;

var i: dynamic;

var j: dynamic;

var x: dynamic;

var y: dynamic;

var ans: dynamic;

var mp = cpp_array((Maxn * 2), Maxn);

func gf(w: dynamic)
{
  if ((f[w] == w))
  {
    return w;
  }
  return cpp_assign(f[w], "=", gf(f[w]));
}

func in_cpp(x: dynamic, y: dynamic)
{
  return (((((x >= 1) && (x <= n)) && (y >= 1)) && (y <= (m * 2))));
}

func fd(p: dynamic, x: dynamic, y: dynamic)
{
  p.clear();
  {
    var i = 0;
    while ((i < 8))
    {
      var cx = (x + sp[i][0]);
      var cy = (y + sp[i][1]);
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
      var a = gf(mp[cx][cy]);
      p.push_back((a));
      i += 1;
    }
  }
}

func ck(x: dynamic, y: dynamic)
{
  var p1: dynamic;
  var p2: dynamic;
  fd(p1, x, y);
  fd(p2, x, (y + m));
  {
    var i = 0;
    while ((i < p1.size()))
    {
      {
        var j = 0;
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

func alt(id: dynamic, x: dynamic, y: dynamic)
{
  mp[x][y] = id;
  {
    var i = 0;
    while ((i < 8))
    {
      var cx = (x + sp[i][0]);
      var cy = (y + sp[i][1]);
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
      var a = gf(mp[cx][cy]);
      f[a] = id;
      i += 1;
    }
  }
}

func main()
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
