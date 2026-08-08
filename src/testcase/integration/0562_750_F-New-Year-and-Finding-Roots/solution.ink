// Translated from solution.cpp.

var gen = cpp_construct(1);

var memo: dynamic;

func ask(x: dynamic)
{
  if (memo.count(x))
  {
    return memo[x];
  }
  write("? ", x, "\n");
  var cnt: dynamic;
  read(cnt);
  for (var el in edges)
  {
    read(el);
  }
  return cpp_assign(memo[x], "=", edges);
}

func check(height: dynamic, k: dynamic)
{
  for (var __cpp_item_1 in height)
  {
    var (v, h) = __cpp_item_1;
    if ((h == k))
    {
      return v;
    } else if (((h + 1) == k))
    {
      var edges = ask(v);
      for (var u in edges)
      {
        var edges_u = ask(u);
        if ((edges_u.size() == 2))
        {
          return u;
        }
      }
    } else if (((h + 2) == k))
    {
      var edges_v = ask(v);
      var cands: dynamic;
      for (var u in edges_v)
      {
        var edges_u = ask(u);
        for (var w in edges_u)
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
        var cand = (*cands.begin());
        cands.erase(cands.begin());
        var resp = ask(cand);
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

func solve()
{
  memo.clear();
  var k: dynamic;
  read(k);
  if ((!k))
  {
    return;
  }
  var first = ((gen() % ((((1 << k)) - 1))) + 1);
  var resp = ask(first);
  var chain = [first];
  var used: dynamic;
  used.insert(first);
  while (true)
  {
    var v = -1;
    for (var u in resp)
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
      var v = -1;
      for (var u in resp)
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
  var height: dynamic;
  {
    var i = 0;
    while ((i < chain.size()))
    {
      var cur_h = (cpp_cast(i) + 1);
      if ((i > (chain.size() / 2)))
      {
        cur_h = cpp_cast(((chain.size() - i)));
      }
      height[chain[i]] = cur_h;
      i += 1;
    }
  }
  assert((chain.size() % 2));
  for (var u in memo[chain[(chain.size() / 2)]])
  {
    if ((!height.count(u)))
    {
      height[u] = (height[chain[(chain.size() / 2)]] + 1);
    }
  }
  while (true)
  {
    var answer = check(height, k);
    if (answer)
    {
      write("! ", answer, "\n");
      return;
    }
    var max_h = -1;
    var v = -1;
    for (var __cpp_item_2 in height)
    {
      var (u, h) = __cpp_item_2;
      if ((h > max_h))
      {
        max_h = h;
        v = u;
      }
    }
    var cur_v = v;
    var path = [cur_v];
    {
      var i = 0;
      while ((i < (max_h - 1)))
      {
        var resp = ask(cur_v);
        if ((resp.size() == 2))
        {
          write("! ", cur_v, "\n");
          return;
        }
        for (var u in resp)
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
    var resp = ask(cur_v);
    if ((resp.size() == 2))
    {
      write("! ", cur_v, "\n");
      return;
    } else if ((resp.size() == 1))
    {
      reverse(path.begin(), path.end());
      {
        var i = 0;
        while ((i < path.size()))
        {
          height[path[i]] = (i + 1);
          i += 1;
        }
      }
      reverse(path.begin(), path.end());
      for (var u in ask(v))
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

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
