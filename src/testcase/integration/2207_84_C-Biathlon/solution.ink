// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

var eps = 1e-8;

var mod = (1e9 + 7);

var P = (1e9 + 7);

var N = 2e7;

var maxn = (1e6 + 5);

var n: dynamic;

var m: dynamic;

var xx: dynamic;

var yy: dynamic;

class node
{
  var x: dynamic;
  var r: dynamic;
  var pos: dynamic;
  var ans: dynamic;
}

var s = cpp_array(10005);

func judge(x: dynamic, y: dynamic, p: dynamic)
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

func cmp1(a: dynamic, b: dynamic)
{
  return (a.x < b.x);
}

func cmp2(a: dynamic, b: dynamic)
{
  return (a.pos < b.pos);
}

func main()
{
  ios.sync_with_stdio(false);
  read(n);
  {
    var i = 0;
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
  var temp = 0;
  {
    var i = 0;
    while ((i < m))
    {
      read(xx, yy);
      var l = 0;
      var r = (n - 1);
      while ((l <= r))
      {
        var mid = (((l + r)) / 2);
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
    var i = 0;
    while ((i < n))
    {
      write(s[i].ans, " ");
      i += 1;
    }
  }
  write("\n");
}
