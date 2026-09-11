// Translated from solution.cpp.

var maxn: dynamic = 2e5;

var a: dynamic = cpp_array((maxn + 11));

var lmin: dynamic = cpp_array((maxn + 11));

var lmax: dynamic = cpp_array((maxn + 11));

var rmin: dynamic = cpp_array((maxn + 11));

var rmax: dynamic = cpp_array((maxn + 11));

var ans3: dynamic = cpp_array(5, (maxn + 11));

var ans4: dynamic = cpp_array(5, (maxn + 11));

var it: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array((maxn + 11));

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, q);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var s: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      while (((!s.empty()) && (a[s.top()] >= a[i])))
      {
        s.pop();
      }
      if (s.empty())
      {
        lmin[i] = 0;
      } else
      {
        lmin[i] = s.top();
      }
      s.push(i);
      i += 1;
    }
  }
  while ((!s.empty()))
  {
    s.pop();
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      while (((!s.empty()) && (a[s.top()] <= a[i])))
      {
        s.pop();
      }
      if (s.empty())
      {
        lmax[i] = 0;
      } else
      {
        lmax[i] = s.top();
      }
      s.push(i);
      i += 1;
    }
  }
  while ((!s.empty()))
  {
    s.pop();
  }
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      while (((!s.empty()) && (a[s.top()] >= a[i])))
      {
        s.pop();
      }
      if (s.empty())
      {
        rmin[i] = (n + 1);
      } else
      {
        rmin[i] = s.top();
      }
      s.push(i);
      i -= 1;
    }
  }
  while ((!s.empty()))
  {
    s.pop();
  }
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      while (((!s.empty()) && (a[s.top()] <= a[i])))
      {
        s.pop();
      }
      if (s.empty())
      {
        rmax[i] = (n + 1);
      } else
      {
        rmax[i] = s.top();
      }
      s.push(i);
      i -= 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var pos: dynamic = max(rmin[i], rmax[i]);
      if ((pos <= n))
      {
        v[pos].emplace_back(i);
      }
      i += 1;
    }
  }
  var lef: dynamic = cpp_uninitialized();
  lef.insert((n + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 0;
        while ((j < 4))
        {
          ans4[i][j] = ans4[(i - 1)][j];
          j += 1;
        }
      }
      for (var pos: dynamic in v[i])
      {
        lef.insert(pos);
      }
      v[i].clear();
      var pos: dynamic = min(lmin[i], lmax[i]);
      it = lef.lower_bound(pos);
      if ((it == lef.begin()))
      {
        i += 1;
        continue;
      }
      it -= 1;
      var x1: dynamic = (*it);
      if ((ans4[i][0] && (x1 <= ans4[i][0])))
      {
        i += 1;
        continue;
      }
      var x2: dynamic =  ((a[rmax[x1]] > a[lmax[i]])) ? rmax[x1] : lmax[i];
      var x3: dynamic =  ((a[rmin[x1]] < a[lmin[i]])) ? rmin[x1] : lmin[i];
      if ((x2 > x3))
      {
        swap(x2, x3);
      }
      ans4[i][0] = x1;
      ans4[i][1] = x2;
      ans4[i][2] = x3;
      ans4[i][3] = i;
      i += 1;
    }
  }
  lef.clear();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((rmax[i] <= n))
      {
        v[rmax[i]].emplace_back(i);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 0;
        while ((j < 3))
        {
          ans3[i][j] = ans3[(i - 1)][j];
          j += 1;
        }
      }
      for (var pos: dynamic in v[i])
      {
        lef.insert(pos);
      }
      v[i].clear();
      var pos: dynamic = lmax[i];
      it = lef.lower_bound(pos);
      if ((it == lef.begin()))
      {
        i += 1;
        continue;
      }
      it -= 1;
      var x1: dynamic = (*it);
      var x2: dynamic =  ((a[rmax[x1]] > a[lmax[i]])) ? rmax[x1] : lmax[i];
      ans3[i][0] = x1;
      ans3[i][1] = x2;
      ans3[i][2] = i;
      i += 1;
    }
  }
  lef.clear();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((rmin[i] <= n))
      {
        v[rmin[i]].emplace_back(i);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((ans3[(i - 1)][0] > ans3[i][0]))
      {
        {
          var j: dynamic = 0;
          while ((j < 3))
          {
            ans3[i][j] = ans3[(i - 1)][j];
            j += 1;
          }
        }
      }
      for (var pos: dynamic in v[i])
      {
        lef.insert(pos);
      }
      v[i].clear();
      var pos: dynamic = lmin[i];
      it = lef.lower_bound(pos);
      if ((it == lef.begin()))
      {
        i += 1;
        continue;
      }
      it -= 1;
      var x1: dynamic = (*it);
      var x2: dynamic =  ((a[rmin[x1]] < a[lmin[i]])) ? rmin[x1] : lmin[i];
      if ((x1 > ans3[i][0]))
      {
        ans3[i][0] = x1;
        ans3[i][1] = x2;
        ans3[i][2] = i;
      }
      i += 1;
    }
  }
  while (cpp_update(q, "--"))
  {
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    read(l, r);
    if ((ans4[r][0] >= l))
    {
      puts("4");
      {
        var i: dynamic = 0;
        while ((i < 4))
        {
          printf("%d ", ans4[r][i]);
          i += 1;
        }
      }
      puts("");
    } else if ((ans3[r][0] >= l))
    {
      puts("3");
      {
        var i: dynamic = 0;
        while ((i < 3))
        {
          printf("%d ", ans3[r][i]);
          i += 1;
        }
      }
      puts("");
    } else
    {
      puts("0");
    }
  }
}
