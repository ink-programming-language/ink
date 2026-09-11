// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          read(a[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          read(b[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var bb: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      bb.push_back(b[i]);
      i += 1;
    }
  }
  sort(bb.begin(), bb.end());
  bb.erase(unique(bb.begin(), bb.end()), bb.end());
  var who_a: dynamic = cpp_construct(bb.size());
  var who_b: dynamic = cpp_construct(bb.size());
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var ind: dynamic = (lower_bound(bb.begin(), bb.end(), a[i]) - bb.begin());
      if (((ind == cpp_cast(bb.size())) || (bb[ind] != a[i])))
      {
        write(-1, "\n");
        return 0;
      }
      who_a[ind].push_back(i);
      ind = (lower_bound(bb.begin(), bb.end(), b[i]) - bb.begin());
      who_b[ind].push_back(i);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(bb.size())))
    {
      if ((who_a[i].size() != who_b[i].size()))
      {
        write(-1, "\n");
        return 0;
      }
      {
        var j: dynamic = 0;
        while ((j < cpp_cast(who_a[i].size())))
        {
          p[who_a[i][j]] = who_b[i][j];
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      q[p[i]] = i;
      i += 1;
    }
  }
  var good: dynamic = cpp_construct(n, 0);
  var cnt_bad: dynamic = cpp_construct(m, 0);
  var cnt_kek: dynamic = cpp_construct(m, 0);
  {
    var j: dynamic = 0;
    while ((j < m))
    {
      {
        var i: dynamic = 0;
        while ((i < (n - 1)))
        {
          if ((a[q[i]][j] < a[q[(i + 1)]][j]))
          {
            cnt_kek[j] += 1;
          }
          if ((a[q[i]][j] > a[q[(i + 1)]][j]))
          {
            cnt_bad[j] += 1;
          }
          i += 1;
        }
      }
      j += 1;
    }
  }
  var ans: dynamic = cpp_uninitialized();
  while (true)
  {
    var ok: dynamic = true;
    {
      var i: dynamic = 0;
      while ((i < (n - 1)))
      {
        ok &= (good[i] || (q[i] < q[(i + 1)]));
        i += 1;
      }
    }
    if (ok)
    {
      break;
    }
    var col: dynamic = -1;
    {
      var j: dynamic = 0;
      while ((j < m))
      {
        if (((cnt_bad[j] == 0) && (cnt_kek[j] > 0)))
        {
          col = j;
          break;
        }
        j += 1;
      }
    }
    if ((col == -1))
    {
      break;
    }
    ans.push_back(col);
    {
      var i: dynamic = 0;
      while ((i < (n - 1)))
      {
        if (((!good[i]) && (a[q[i]][col] < a[q[(i + 1)]][col])))
        {
          good[i] = true;
          {
            var j: dynamic = 0;
            while ((j < m))
            {
              if ((a[q[i]][j] < a[q[(i + 1)]][j]))
              {
                cnt_kek[j] -= 1;
              }
              if ((a[q[i]][j] > a[q[(i + 1)]][j]))
              {
                cnt_bad[j] -= 1;
              }
              j += 1;
            }
          }
        }
        i += 1;
      }
    }
  }
  var ok: dynamic = true;
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      ok &= (good[i] || (q[i] < q[(i + 1)]));
      i += 1;
    }
  }
  if (ok)
  {
    write(cpp_cast(ans.size()), "\n");
    reverse(ans.begin(), ans.end());
    for (var i: dynamic in ans)
    {
      write((i + 1), " ");
    }
    write("\n");
  } else
  {
    write(-1, "\n");
  }
  return 0;
}
