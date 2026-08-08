// Translated from solution.cpp.

func TEN(n: dynamic)
{
  return if (((n == 0))) 1 else (10 * TEN((n - 1)));
}

func ok(v: dynamic)
{
  for (var p in v)
  {
    if ((p.first > p.second))
    {
      swap(p.first, p.second);
    }
  }
  sort(begin(v), end(v));
  var st: dynamic;
  for (var p in v)
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

func main()
{
  var h: dynamic;
  var w: dynamic;
  scanf("%d %d", (&h), (&w));
  var n = (h * w);
  var rid = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      var a: dynamic;
      read(a);
      rid[a] = i;
      i += 1;
    }
  }
  {
    var y = 0;
    while ((y < h))
    {
      var v = cpp_array(2);
      {
        var x = 0;
        while ((x < (w - 1)))
        {
          var id = ((y * w) + x);
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
    var x = 0;
    while ((x < w))
    {
      var v = cpp_array(2);
      {
        var y = 0;
        while ((y < (h - 1)))
        {
          var id = ((y * w) + x);
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
