// Translated from solution.cpp.

var b = cpp_array(500005);

var ans = cpp_array(500005);

class Base
{
  var a: dynamic = cpp_array(23);
  var pos: dynamic = cpp_array(23);
  func Base()
  {
      {
        var i = 0;
        while ((i < 23))
        {
          a[i] = 0;
          i += 1;
        }
      }
    }
  func up(a: dynamic, b: dynamic)
  {
      if ((b > a))
      {
        a = b;
      }
    }
  func ins(x: dynamic, r: dynamic)
  {
      {
        var i = 22;
        while ((~i))
        {
          if (((x >> i) & 1))
          {
            if (a[i])
            {
              if ((pos[i] < r))
              {
                swap(pos[i], r);
                swap(a[i], x);
              }
              x ^= a[i];
            } else
            {
              a[i] = x;
              pos[i] = r;
              break;
            }
          }
          i -= 1;
        }
      }
    }
  func ask(r: dynamic)
  {
      var t = 0;
      {
        var i = 22;
        while ((~i))
        {
          if ((pos[i] >= r))
          {
            up(t, (t ^ a[i]));
          }
          i -= 1;
        }
      }
      return t;
    }
}

var f: dynamic;

class node
{
  var l: dynamic;
  var r: dynamic;
  var idx: dynamic;
  func operator_less(a: dynamic)
  {
      if ((r == a.r))
      {
        return (l < a.l);
      }
      return (r < a.r);
    }
}

var e = cpp_array(500005);

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&b[i]));
      i += 1;
    }
  }
  var q: dynamic;
  scanf("%d", (&q));
  {
    var i = 1;
    while ((i <= q))
    {
      scanf("%d%d", (&e[i].l), (&e[i].r));
      e[i].idx = i;
      i += 1;
    }
  }
  sort((e + 1), ((e + 1) + q));
  var r = 0;
  {
    var i = 1;
    while ((i <= q))
    {
      while (((r < n) && (r < e[i].r)))
      {
        r += 1;
        f.ins(b[r], r);
      }
      ans[e[i].idx] = f.ask(e[i].l);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= q))
    {
      printf("%d\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
