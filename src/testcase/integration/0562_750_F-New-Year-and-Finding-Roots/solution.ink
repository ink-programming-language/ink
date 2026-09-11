// Translated from solution.cpp.

var gen: dynamic = cpp_construct(1);

var memo: dynamic = cpp_uninitialized();

func ask(x: dynamic) -> dynamic
{
  if (memo.count(x))
  {
    return memo[x];
  }
  write("? ", x, "\n");
  var cnt: dynamic = cpp_uninitialized();
  read(cnt);
  for (var el: dynamic in edges)
  {
    read(el);
  }
  return cpp_assign(memo[x], "=", edges);
}

func check(height: dynamic, k: dynamic) -> dynamic
{
  for (var __cpp_item_1: dynamic in height)
  {
    var (v, h): dynamic = __cpp_item_1;
    if ((h == k))
    {
      return v;
    } else if (((h + 1) == k))
    {
      var edges: dynamic = ask(v);
      for (var u: dynamic in edges)
      {
        var edges_u: dynamic = ask(u);
        if ((edges_u.size() == 2))
        {
          return u;
        }
      }
    } else if (((h + 2) == k))
    {
      var edges_v: dynamic = ask(v);
      var cands: dynamic = cpp_uninitialized();
      for (var u: dynamic in edges_v)
      {
        var edges_u: dynamic = ask(u);
        for (var w: dynamic in edges_u)
        {
          if (memo.count(w))
          {
            if ((memo[w].size() == 2))
            {
              return w;
            }
          } else if ((w != v))
          {
            cands.insert(w);
          }
        }
      }
      assert((cands.size() > 0));
      while ((cands.size() > 1))
      {
        var cand: dynamic = (*cands.begin());
        cands.erase(cands.begin());
        var resp: dynamic = ask(cand);
        if ((resp.size() == 2))
        {
          return cand;
        }
      }
      return (*cands.begin());
    }
  }
  return 0;
}

func solve() -> dynamic
{
  memo.clear();
  var k: dynamic = cpp_uninitialized();
  read(k);
  if ((!k))
  {
    return;
  }
  var first: dynamic = ((gen() % ((((1 << k)) - 1))) + 1);
  var resp: dynamic = ask(first);
  var chain: dynamic = [first];
  var used: dynamic = cpp_uninitialized();
  used.insert(first);
  while (true)
  {
    var v: dynamic = -1;
    for (var u: dynamic in resp)
    {
      if ((!used.count(u)))
      {
        v = u;
        used.insert(v);
        break;
      }
    }
    chain.push_back(v);
    resp = ask(v);
    if ((resp.size() == 2))
    {
      write("! ", v, "\n");
      return;
    }
    if ((resp.size() == 1))
    {
      break;
    }
  }
  if ((memo[first].size() > 1))
  {
    reverse(chain.begin(), chain.end());
    resp = memo[first];
    while (true)
    {
      var v: dynamic = -1;
      for (var u: dynamic in resp)
      {
        if ((!used.count(u)))
        {
          v = u;
          used.insert(v);
          break;
        }
      }
      chain.push_back(v);
      resp = ask(v);
      if ((resp.size() == 2))
      {
        write("! ", v, "\n");
        return;
      }
      if ((resp.size() == 1))
      {
        break;
      }
    }
  }
  var height: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < chain.size()))
    {
      var cur_h: dynamic = (cpp_cast(i) + 1);
      if ((i > (chain.size() / 2)))
      {
        cur_h = cpp_cast(((chain.size() - i)));
      }
      height[chain[i]] = cur_h;
      i += 1;
    }
  }
  assert((chain.size() % 2));
  for (var u: dynamic in memo[chain[(chain.size() / 2)]])
  {
    if ((!height.count(u)))
    {
      height[u] = (height[chain[(chain.size() / 2)]] + 1);
    }
  }
  while (true)
  {
    var answer: dynamic = check(height, k);
    if (answer)
    {
      write("! ", answer, "\n");
      return;
    }
    var max_h: dynamic = -1;
    var v: dynamic = -1;
    for (var __cpp_item_2: dynamic in height)
    {
      var (u, h): dynamic = __cpp_item_2;
      if ((h > max_h))
      {
        max_h = h;
        v = u;
      }
    }
    var cur_v: dynamic = v;
    var path: dynamic = [cur_v];
    {
      var i: dynamic = 0;
      while ((i < (max_h - 1)))
      {
        var resp: dynamic = ask(cur_v);
        if ((resp.size() == 2))
        {
          write("! ", cur_v, "\n");
          return;
        }
        for (var u: dynamic in resp)
        {
          if (((!height.count(u)) && (((path.size() < 2) || (u != path[(cpp_cast(path.size()) - 2)])))))
          {
            cur_v = u;
            path.push_back(cur_v);
            break;
          }
        }
        i += 1;
      }
    }
    var resp: dynamic = ask(cur_v);
    if ((resp.size() == 2))
    {
      write("! ", cur_v, "\n");
      return;
    } else if ((resp.size() == 1))
    {
      reverse(path.begin(), path.end());
      {
        var i: dynamic = 0;
        while ((i < path.size()))
        {
          height[path[i]] = (i + 1);
          i += 1;
        }
      }
      reverse(path.begin(), path.end());
      for (var u: dynamic in ask(v))
      {
        if (((!height.count(u)) && (u != path[1])))
        {
          height[u] = (height[v] + 1);
          break;
        }
      }
    } else
    {
      height[path[1]] = (height[v] + 1);
    }
  }
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
