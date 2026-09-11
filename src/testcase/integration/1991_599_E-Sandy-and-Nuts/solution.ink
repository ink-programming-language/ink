// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array((1 << 15), 15);

var con: dynamic = cpp_array(15);

var lca: dynamic = cpp_array(105);

func myinit() -> dynamic
{
  read(n, m, q);
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      u -= 1;
      v -= 1;
      con[i] = make_pair(u, v);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= q))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var w: dynamic = cpp_uninitialized();
      read(u, v, w);
      u -= 1;
      v -= 1;
      w -= 1;
      lca[i] = make_pair(make_pair(u, v), w);
      i += 1;
    }
  }
}

func dfs(root: dynamic, mask: dynamic) -> dynamic
{
  if (dp[root][mask])
  {
    return;
  }
  var maskll: dynamic = mask;
  mask -= (1 << root);
  if ((mask == 0))
  {
    dp[root][maskll] = 1;
    return;
  }
  var least: dynamic = ((mask) - (((mask) & (((mask) - 1)))));
  var newmask: dynamic = mask;
  var newroot: dynamic = cpp_uninitialized();
  {
    newmask = mask;
    while ((newmask != 0))
    {
      if ((!((newmask & least))))
      {
        newmask = (((newmask - 1)) & mask);
        continue;
      }
      var was: dynamic = true;
      var cnt: dynamic = 0;
      {
        var i: dynamic = 1;
        while ((i <= m))
        {
          var u: dynamic = cpp_uninitialized();
          var v: dynamic = cpp_uninitialized();
          u = con[i].first;
          v = con[i].second;
          if (((!(((maskll) & ((1 << u))))) || (!(((maskll) & ((1 << v)))))))
          {
            i += 1;
            continue;
          }
          if ((u == root))
          {
            swap(u, v);
          }
          if ((((newmask & ((1 << u))) && (newmask & ((1 << v)))) || ((((maskll - newmask)) & ((1 << u))) && (((maskll - newmask)) & ((1 << v))))))
          {
            i += 1;
            continue;
          }
          if (((v == root) && (newmask & ((1 << u)))))
          {
            cnt += 1;
            newroot = u;
            i += 1;
            continue;
          }
          was = false;
          i += 1;
        }
      }
      if (((cnt > 1) || (!was)))
      {
        newmask = (((newmask - 1)) & mask);
        continue;
      }
      {
        var i: dynamic = 1;
        while ((i <= q))
        {
          var u: dynamic = cpp_uninitialized();
          var v: dynamic = cpp_uninitialized();
          var w: dynamic = cpp_uninitialized();
          u = lca[i].first.first;
          v = lca[i].first.second;
          w = lca[i].second;
          if ((((!(((maskll) & ((1 << u))))) || (!(((maskll) & ((1 << v)))))) || (!(((maskll) & ((1 << w)))))))
          {
            i += 1;
            continue;
          }
          if ((w == root))
          {
            if (((u == root) || (v == root)))
            {
              i += 1;
              continue;
            }
            if (((newmask & ((1 << u))) && (newmask & ((1 << v)))))
            {
              was = false;
              break;
            }
          } else
          {
            var t: dynamic = 0;
            if ((newmask & ((1 << u))))
            {
              t += 1;
            }
            if ((newmask & ((1 << v))))
            {
              t += 1;
            }
            if ((newmask & ((1 << w))))
            {
              t += 1;
            }
            if (((t != 0) && (t != 3)))
            {
              was = false;
            }
          }
          i += 1;
        }
      }
      if (was)
      {
        if ((cnt == 1))
        {
          dfs(newroot, newmask);
          dfs(root, (maskll - newmask));
          dp[root][maskll] += (dp[newroot][newmask] * dp[root][(maskll - newmask)]);
        } else
        {
          {
            var i: dynamic = 0;
            while ((i < n))
            {
              if ((newmask & ((1 << i))))
              {
                newroot = i;
                dfs(newroot, newmask);
                dfs(root, (maskll - newmask));
                dp[root][maskll] += (dp[newroot][newmask] * dp[root][(maskll - newmask)]);
              }
              i += 1;
            }
          }
        }
      }
      newmask = (((newmask - 1)) & mask);
    }
  }
}

func main() -> dynamic
{
  myinit();
  dfs(0, (((1 << n)) - 1));
  write(dp[0][(((1 << n)) - 1)]);
  return 0;
}
