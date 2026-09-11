// Translated from solution.cpp.

var INF: dynamic = 0x3f3f3f3f;

var eps: dynamic = 1e-8;

var mod: dynamic = (1e9 + 7);

var P: dynamic = (1e9 + 7);

var N: dynamic = 2e7;

var maxn: dynamic = (1e6 + 5);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var xx: dynamic = cpp_uninitialized();

var yy: dynamic = cpp_uninitialized();

class node
{
  var x: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var pos: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
}

var s: dynamic = cpp_array(10005);

func judge(x: dynamic, y: dynamic, p: dynamic) -> dynamic
{
  if (((p < 0) || (p >= n)))
  {
    return false;
  }
  if ((s[p].ans != -1))
  {
    return false;
  }
  if ((((((s[p].x - x)) * ((s[p].x - x))) + (y * y)) <= (s[p].r * s[p].r)))
  {
    return true;
  }
  return false;
}

func cmp1(a: dynamic, b: dynamic) -> dynamic
{
  return (a.x < b.x);
}

func cmp2(a: dynamic, b: dynamic) -> dynamic
{
  return (a.pos < b.pos);
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(s[i].x, s[i].r);
      s[i].pos = (i + 1);
      s[i].ans = -1;
      i += 1;
    }
  }
  sort(s, (s + n), cmp1);
  read(m);
  var temp: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(xx, yy);
      var l: dynamic = 0;
      var r: dynamic = (n - 1);
      while ((l <= r))
      {
        var mid: dynamic = (((l + r)) / 2);
        if ((s[mid].x > xx))
        {
          r = (mid - 1);
        } else
        {
          l = (mid + 1);
        }
      }
      if (judge(xx, yy, (l - 1)))
      {
        s[(l - 1)].ans = (i + 1);
        temp += 1;
      }
      if (judge(xx, yy, l))
      {
        s[l].ans = (i + 1);
        temp += 1;
      }
      if (judge(xx, yy, (l + 1)))
      {
        s[(l + 1)].ans = (i + 1);
        temp += 1;
      }
      i += 1;
    }
  }
  sort(s, (s + n), cmp2);
  write(temp, "\n");
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(s[i].ans, " ");
      i += 1;
    }
  }
  write("\n");
}
