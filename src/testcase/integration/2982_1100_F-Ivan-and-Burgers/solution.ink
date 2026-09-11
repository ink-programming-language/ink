// Translated from solution.cpp.

var b: dynamic = cpp_array(500005);

var ans: dynamic = cpp_array(500005);

class Base
{
  var a: dynamic = cpp_array(23);
  var pos: dynamic = cpp_array(23);
  func Base() -> dynamic
  {
      {
        var i: dynamic = 0;
        while ((i < 23))
        {
          a[i] = 0;
          i += 1;
        }
      }
    }
  func up(a: dynamic, b: dynamic) -> dynamic
  {
      if ((b > a))
      {
        a = b;
      }
    }
  func ins(x: dynamic, r: dynamic) -> dynamic
  {
      {
        var i: dynamic = 22;
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
  func ask(r: dynamic) -> dynamic
  {
      var t: dynamic = 0;
      {
        var i: dynamic = 22;
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

var f: dynamic = cpp_uninitialized();

class node
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var idx: dynamic = cpp_uninitialized();
  func operator_less(a: dynamic) -> dynamic
  {
      if ((r == a.r))
      {
        return (l < a.l);
      }
      return (r < a.r);
    }
}

var e: dynamic = cpp_array(500005);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&b[i]));
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  scanf("%d", (&q));
  {
    var i: dynamic = 1;
    while ((i <= q))
    {
      scanf("%d%d", (&e[i].l), (&e[i].r));
      e[i].idx = i;
      i += 1;
    }
  }
  sort((e + 1), ((e + 1) + q));
  var r: dynamic = 0;
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= q))
    {
      printf("%d\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
