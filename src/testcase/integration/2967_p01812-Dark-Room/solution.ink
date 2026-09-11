// Translated from solution.cpp.

class hash_std_pair_longlong_longlong
{
  func operator_call(x: dynamic) -> dynamic
  {
      return (hash()(x.first) ^ hash()(x.second));
    }
}

class hash_std_pair_RoomInfo_int
{
  func operator_call(x: dynamic) -> dynamic
  {
      return (hash()(x.first) ^ hash()(x.second));
    }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var e: dynamic = cpp_array(100, 100);
  var q: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
  var visited: dynamic = cpp_uninitialized();
  read(n, m, k);
  if ((m == 0))
  {
    write(0, "\n");
    return 0;
  }
  d.resize(n, 0);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var a: dynamic = cpp_uninitialized();
      read(a);
      d[(a - 1)] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < k))
        {
          var a: dynamic = cpp_uninitialized();
          read(a);
          e[i][j] = (a - 1);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var node: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
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
    var node: dynamic = q.front();
    q.pop();
    node.second += 1;
    {
      var i: dynamic = 0;
      while ((i < k))
      {
        var next: dynamic = make_pair(0, 0);
        {
          var j: dynamic = 0;
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
            var to: dynamic = e[j][i];
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
