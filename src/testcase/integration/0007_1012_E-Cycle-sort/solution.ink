// Translated from solution.cpp.

var MAXN = 300000;

var mapa: dynamic;

var pos: dynamic;

var g = cpp_array(MAXN);

var ptr = cpp_array(MAXN);

var used = cpp_array(MAXN);

func euler(v: dynamic, res: dynamic)
{
  used[v] = true;
  {
    while ((ptr[v] < cpp_cast((g[v]).size())))
    {
      ptr[v] += 1;
      var u = g[v][(ptr[v] - 1)];
      euler(u, res);
      res.push_back(u);
    }
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic;
  var s: dynamic;
  read(n, s);
  var k = 0;
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var b = a;
  sort((b).begin(), (b).end());
  var m = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((a[i] == b[i]))
      {
        i += 1;
        continue;
      }
      m += 1;
      if ((!mapa.count(b[i])))
      {
        mapa[b[i]] = cpp_update(k, "++");
      }
      i += 1;
    }
  }
  if ((m > s))
  {
    write(-1, "\n");
    return 0;
  }
  {
    var i = 0;
    while ((i < n))
    {
      if ((a[i] == b[i]))
      {
        i += 1;
        continue;
      }
      a[i] = mapa[a[i]];
      b[i] = mapa[b[i]];
      g[b[i]].push_back(a[i]);
      pos[[b[i], a[i]]].push_back(i);
      i += 1;
    }
  }
  var cycles: dynamic;
  {
    var i = 0;
    while ((i < k))
    {
      if ((!used[i]))
      {
        var arr: dynamic;
        euler(i, arr);
        reverse((arr).begin(), (arr).end());
        cycles.push_back([]);
        {
          var i = 0;
          while ((i < cpp_cast((arr).size())))
          {
            var j = (((i + 1)) % cpp_cast((arr).size()));
            cycles.back().push_back(pos[[arr[i], arr[j]]].back());
            pos[[arr[i], arr[j]]].pop_back();
            i += 1;
          }
        }
      }
      i += 1;
    }
  }
  var res: dynamic;
  if ((((s - m) > 1) && (cpp_cast((cycles).size()) > 1)))
  {
    var len = min(cpp_cast((cycles).size()), (s - m));
    res.push_back([]);
    var newcycle: dynamic;
    {
      var i = (cpp_cast((cycles).size()) - len);
      while ((i < cpp_cast((cycles).size())))
      {
        res.back().push_back(cycles[i].back());
        for (var j in cycles[i])
        {
          newcycle.push_back(j);
        }
        i += 1;
      }
    }
    reverse((res.back()).begin(), (res.back()).end());
    {
      var i = 0;
      while ((i < len))
      {
        cycles.pop_back();
        i += 1;
      }
    }
    cycles.push_back(newcycle);
  }
  {
    var i = 0;
    while ((i < cpp_cast((cycles).size())))
    {
      res.push_back(cycles[i]);
      i += 1;
    }
  }
  write(cpp_cast((res).size()), "\n");
  {
    var i = 0;
    while ((i < cpp_cast((res).size())))
    {
      write(cpp_cast((res[i]).size()), "\n");
      for (var j in res[i])
      {
        write((j + 1), " ");
      }
      write("\n");
      i += 1;
    }
  }
}
