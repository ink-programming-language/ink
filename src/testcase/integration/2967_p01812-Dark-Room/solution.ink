// Translated from solution.cpp.

class hash_std_pair_longlong_longlong
{
  func operator_call(x: dynamic)
  {
      return (hash()(x.first) ^ hash()(x.second));
    }
}

class hash_std_pair_RoomInfo_int
{
  func operator_call(x: dynamic)
  {
      return (hash()(x.first) ^ hash()(x.second));
    }
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  var d: dynamic;
  var e = cpp_array(100, 100);
  var q: dynamic;
  var g: dynamic;
  var visited: dynamic;
  read(n, m, k);
  if ((m == 0))
  {
    write(0, "\n");
    return 0;
  }
  d.resize(n, 0);
  {
    var i = 0;
    while ((i < m))
    {
      var a: dynamic;
      read(a);
      d[(a - 1)] = 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < k))
        {
          var a: dynamic;
          read(a);
          e[i][j] = (a - 1);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var node: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      if (d[i])
      {
        if ((i < 64))
        {
          node.first |= (1 << i);
        } else
        {
          node.second |= (1 << ((i - 64)));
        }
      }
      i += 1;
    }
  }
  q.push(make_pair(node, 0));
  visited.insert(node);
  while ((!q.empty()))
  {
    var node = q.front();
    q.pop();
    node.second += 1;
    {
      var i = 0;
      while ((i < k))
      {
        var next = make_pair(0, 0);
        {
          var j = 0;
          while ((j < n))
          {
            if ((j < 64))
            {
              if ((!((node.first.first & (1 << j)))))
              {
                j += 1;
                continue;
              }
            } else
            {
              if ((!((node.first.second & (1 << ((j - 64)))))))
              {
                j += 1;
                continue;
              }
            }
            var to = e[j][i];
            if (d[to])
            {
              if ((to < 64))
              {
                next.first |= (1 << to);
              } else
              {
                next.second |= (1 << ((to - 64)));
              }
            }
            j += 1;
          }
        }
        if (((!next.first) && (!next.second)))
        {
          write(node.second, "\n");
          return 0;
        }
        if ((!visited.count(next)))
        {
          q.push(make_pair(next, node.second));
          visited.insert(next);
        }
        i += 1;
      }
    }
  }
  return 0;
}
