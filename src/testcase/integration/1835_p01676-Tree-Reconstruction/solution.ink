// Translated from solution.cpp.

class edge
{
  var to: dynamic;
  var from_cpp: dynamic;
  func edge(to: dynamic, from_cpp: dynamic)
  {
      this->to = cpp_construct(to);
      this->from_cpp = cpp_construct(from_cpp);
    }
  func edge()
  {
    }
}

var N: dynamic;

var M: dynamic;

var used = cpp_array(5000);

var G = cpp_array(555);

var E: dynamic;

func main()
{
  read(N, M);
  {
    var i = 0;
    while ((i < M))
    {
      var s: dynamic;
      var t: dynamic;
      read(s, t);
      s -= 1;
      t -= 1;
      E.push_back(edge(t, s));
      G[s].insert(i);
      G[t].insert(i);
      i += 1;
    }
  }
  if ((M == 0))
  {
    write(0, "\n");
    return 0;
  }
  var ed = true;
  var res = 0;
  while (ed)
  {
    var minid = 0;
    while (G[minid].empty())
    {
      minid += 1;
    }
    {
      var i = 0;
      while ((i < N))
      {
        if (G[i].empty())
        {
          i += 1;
          continue;
        }
        if ((G[minid].size() > G[i].size()))
        {
          minid = i;
        }
        i += 1;
      }
    }
    res += (G[minid].size() - 1);
    {
      var it = G[minid].begin();
      while ((it != G[minid].end()))
      {
        used[(*it)] = true;
        {
          var i = 0;
          while ((i < N))
          {
            if ((i == minid))
            {
              i += 1;
              continue;
            }
            G[i].erase((*it));
            i += 1;
          }
        }
        it += 1;
      }
    }
    G[minid].clear();
    ed = false;
    {
      var i = 0;
      while ((i < M))
      {
        if ((!used[i]))
        {
          ed = true;
        }
        i += 1;
      }
    }
  }
  write(res, "\n");
}
