// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var t: dynamic;

var k: dynamic;

var pos = cpp_array(200005);

var tmp = cpp_array(200005);

var a = cpp_array(200005);

class dsu
{
  var fa: dynamic = cpp_array(200005);
  func init(n: dynamic)
  {
      {
        var i = 0;
        while ((i <= n))
        {
          fa[i] = i;
          i += 1;
        }
      }
    }
  func find(u: dynamic)
  {
      while ((u != fa[u]))
      {
        u = cpp_assign(fa[u], "=", fa[fa[u]]);
      }
      fa[u] = (u + 1);
      return u;
    }
}

var num: dynamic;

func cmp(a: dynamic, b: dynamic)
{
  if ((a.first.second != b.first.second))
  {
    return (a.first.second < b.first.second);
  }
  return (a.first.first < b.first.first);
}

var mp: dynamic;

var points: dynamic;

func print(p: dynamic)
{
  {
    var i = 1;
    while ((i <= n))
    {
      write(p[i], " ");
      i += 1;
    }
  }
  write("\n");
}

func swp(a: dynamic, b: dynamic)
{
  {
    var i = 1;
    while ((i <= n))
    {
      tmp[i] = pos[i];
      if ((tmp[i] == a))
      {
        tmp[i] = b;
      } else if ((tmp[i] == b))
      {
        tmp[i] = a;
      }
      i += 1;
    }
  }
}

func cmp2(a: dynamic, b: dynamic)
{
  if ((a.first.first != b.first.first))
  {
    return (a.first.first < b.first.first);
  }
  return (a.second < b.second);
}

func main()
{
  ios.sync_with_stdio(0);
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i].first.first, a[i].first.second);
      a[i].second = (i + 1);
      i += 1;
    }
  }
  sort(a, (a + n), cmp);
  num.init(n);
  var sud = 1;
  {
    var i = 0;
    while ((i < n))
    {
      pos[a[i].second] = num.find(a[i].first.first);
      points.push_back([make_pair(pos[a[i].second], a[i].first.first), 0]);
      points.push_back([make_pair(a[i].first.second, a[i].first.first), pos[a[i].second]]);
      i += 1;
    }
  }
  sort(points.begin(), points.end(), cmp2);
  for (var point in points)
  {
    if ((point.second == 0))
    {
      if (((!mp.empty()) && (point.first.second <= ((*mp.rbegin())))))
      {
        swp(((*mp.rbegin())), point.first.first);
        sud = 0;
        break;
      }
      mp.insert(point.first.first);
    } else
    {
      mp.erase(point.second);
    }
  }
  if (sud)
  {
    write("YES\n");
    print(pos);
  } else
  {
    write("NO\n");
    print(pos);
    print(tmp);
  }
  return 0;
}
