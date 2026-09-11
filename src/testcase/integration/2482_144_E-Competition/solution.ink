// Translated from solution.cpp.

var q: dynamic = cpp_uninitialized();

class node
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var p: dynamic = cpp_array(100010);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return ((cpp_cast(a))->l - (cpp_cast(b))->l);
}

var ans: dynamic = cpp_array(100010);

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      scanf("%d%d", (&x), (&y));
      p[i].l = ((n + 1) - y);
      p[i].r = x;
      p[i].id = i;
      i += 1;
    }
  }
  qsort((p + 1), m, cpp_sizeof((node)), cmp);
  var j: dynamic = 1;
  var cnt: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      while (((j <= m) && (p[j].l <= i)))
      {
        q.push(make_pair((-p[j].r), p[j].id));
        j += 1;
      }
      while ((!q.empty()))
      {
        var t: dynamic = q.top();
        q.pop();
        if (((-t.first) >= i))
        {
          cnt += 1;
          ans[cnt] = t.second;
          break;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", cnt);
  {
    var i: dynamic = 1;
    while ((i <= cnt))
    {
      printf("%d ", ans[i]);
      i += 1;
    }
  }
}
