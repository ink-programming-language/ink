// Translated from solution.cpp.

func TEN(n: dynamic) -> dynamic
{
  return  (((n == 0))) ? 1 : (10 * TEN((n - 1)));
}

func ok(v: dynamic) -> dynamic
{
  for (var p: dynamic in v)
  {
    if ((p.first > p.second))
    {
      swap(p.first, p.second);
    }
  }
  sort(begin(v), end(v));
  var st: dynamic = cpp_uninitialized();
  for (var p: dynamic in v)
  {
    while ((st.size() && (st.back().second < p.first)))
    {
      st.pop_back();
    }
    if ((st.size() && (st.back().second < p.second)))
    {
      return false;
    }
    st.push_back(p);
  }
  return true;
}

func main() -> dynamic
{
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  scanf("%d %d", (&h), (&w));
  var n: dynamic = (h * w);
  var rid: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      read(a);
      rid[a] = i;
      i += 1;
    }
  }
  {
    var y: dynamic = 0;
    while ((y < h))
    {
      var v: dynamic = cpp_array(2);
      {
        var x: dynamic = 0;
        while ((x < (w - 1)))
        {
          var id: dynamic = ((y * w) + x);
          v[(x % 2)].push_back(P(rid[id], rid[(id + 1)]));
          x += 1;
        }
      }
      if (((!ok(v[0])) || (!ok(v[1]))))
      {
        write("NO", "\n");
        return 0;
      }
      y += 1;
    }
  }
  {
    var x: dynamic = 0;
    while ((x < w))
    {
      var v: dynamic = cpp_array(2);
      {
        var y: dynamic = 0;
        while ((y < (h - 1)))
        {
          var id: dynamic = ((y * w) + x);
          v[(y % 2)].push_back(P(rid[id], rid[(id + w)]));
          y += 1;
        }
      }
      if (((!ok(v[0])) || (!ok(v[1]))))
      {
        write("NO", "\n");
        return 0;
      }
      x += 1;
    }
  }
  write("YES", "\n");
  return 0;
}
