// Translated from solution.cpp.

var MAXN: dynamic = 300000;

var mapa: dynamic = cpp_uninitialized();

var pos: dynamic = cpp_uninitialized();

var g: dynamic = cpp_array(MAXN);

var cpp_ptr: dynamic = cpp_array(MAXN);

var used: dynamic = cpp_array(MAXN);

func euler(v: dynamic, res: dynamic) -> dynamic
{
  used[v] = true;
  {
    while ((cpp_ptr[v] < cpp_cast((g[v]).size())))
    {
      cpp_ptr[v] += 1;
      var u: dynamic = g[v][(cpp_ptr[v] - 1)];
      euler(u, res);
      res.push_back(u);
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, s);
  var k: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var b: dynamic = a;
  sort((b).begin(), (b).end());
  var m: dynamic = 0;
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
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
  var cycles: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      if ((!used[i]))
      {
        var arr: dynamic = cpp_uninitialized();
        euler(i, arr);
        reverse((arr).begin(), (arr).end());
        cycles.push_back([]);
        {
          var i: dynamic = 0;
          while ((i < cpp_cast((arr).size())))
          {
            var j: dynamic = (((i + 1)) % cpp_cast((arr).size()));
            cycles.back().push_back(pos[[arr[i], arr[j]]].back());
            pos[[arr[i], arr[j]]].pop_back();
            i += 1;
          }
        }
      }
      i += 1;
    }
  }
  var res: dynamic = cpp_uninitialized();
  if ((((s - m) > 1) && (cpp_cast((cycles).size()) > 1)))
  {
    var len: dynamic = min(cpp_cast((cycles).size()), (s - m));
    res.push_back([]);
    var newcycle: dynamic = cpp_uninitialized();
    {
      var i: dynamic = (cpp_cast((cycles).size()) - len);
      while ((i < cpp_cast((cycles).size())))
      {
        res.back().push_back(cycles[i].back());
        for (var j: dynamic in cycles[i])
        {
          newcycle.push_back(j);
        }
        i += 1;
      }
    }
    reverse((res.back()).begin(), (res.back()).end());
    {
      var i: dynamic = 0;
      while ((i < len))
      {
        cycles.pop_back();
        i += 1;
      }
    }
    cycles.push_back(newcycle);
  }
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((cycles).size())))
    {
      res.push_back(cycles[i]);
      i += 1;
    }
  }
  write(cpp_cast((res).size()), "\n");
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((res).size())))
    {
      write(cpp_cast((res[i]).size()), "\n");
      for (var j: dynamic in res[i])
      {
        write((j + 1), " ");
      }
      write("\n");
      i += 1;
    }
  }
}
