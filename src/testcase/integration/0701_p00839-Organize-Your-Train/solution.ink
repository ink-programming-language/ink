// Translated from solution.cpp.

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func rolling_hash(s: dynamic) -> dynamic
{
  var base: dynamic = 1000000007;
  var res: dynamic = 0;
  for (var c: dynamic in s)
  {
    res = ((res * base) + c);
  }
  return res;
}

func to_str(state: dynamic) -> dynamic
{
  var res: dynamic = "";
  for (var s: dynamic in state)
  {
    res += (s + ":");
  }
  return res;
}

func bfs(limit: dynamic, start: dynamic, exchanges: dynamic) -> dynamic
{
  var res: dynamic = cpp_uninitialized();
  var que: dynamic = cpp_uninitialized();
  res.insert([rolling_hash(to_str(start)), 0]);
  que.push(start);
  while ((!que.empty()))
  {
    var state: dynamic = que.front();
    que.pop();
    var d: dynamic = res.at(rolling_hash(to_str(state)));
    if ((d == limit))
    {
      break;
    }
    for (var element: dynamic in exchanges)
    {
      var pos: dynamic = element.first;
      var dir: dynamic = element.second;
      var train: dynamic = [move(state[pos.front()]), move(state[pos.back()])];
      {
        var i: dynamic = 0;
        while ((i <= 1))
        {
          var from_cpp: dynamic = i;
          var to: dynamic = (1 - i);
          var p1: dynamic = pos[from_cpp];
          var p2: dynamic = pos[to];
          var d1: dynamic = dir[from_cpp];
          var d2: dynamic = dir[to];
          var t1: dynamic = train[from_cpp];
          var t2: dynamic = train[to];
          {
            var num: dynamic = 1;
            while ((num <= t1.size()))
            {
              var tmp: dynamic = ( (d1) ? t1.substr(0, num) : t1.substr((t1.size() - num)));
              if ((d1 == d2))
              {
                reverse(tmp.begin(), tmp.end());
              }
              state[p1] = ( (d1) ? t1.substr(num) : t1.substr(0, (t1.size() - num)));
              state[p2] = ( (d2) ? (tmp + t2) : (t2 + tmp));
              var h: dynamic = rolling_hash(to_str(state));
              if ((!res.count(h)))
              {
                res.insert([h, (d + 1)]);
                que.push(state);
              }
              num += 1;
            }
          }
          i += 1;
        }
      }
      state[pos.front()] = move(train.front());
      state[pos.back()] = move(train.back());
    }
  }
  return res;
}

func main() -> dynamic
{
  cin.tie(null);
  ios.sync_with_stdio(false);
  {
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    while ((((cin >> x) >> y) && x))
    {
      var exchanges: dynamic = cpp_uninitialized();
      exchanges.reserve(y);
      {
        var i: dynamic = 0;
        while ((i < y))
        {
          var a: dynamic = cpp_uninitialized();
          var b: dynamic = cpp_uninitialized();
          read(a, b);
          var p1: dynamic = (a[0] - cpp_char("0"));
          var d1: dynamic = ((a[1] == cpp_char("W")));
          var p2: dynamic = (b[0] - cpp_char("0"));
          var d2: dynamic = ((b[1] == cpp_char("W")));
          exchanges.emplace_back([p1, p2], [d1, d2]);
          i += 1;
        }
      }
      for (var e: dynamic in lines)
      {
        read(e);
        if ((e == "-"))
        {
          e = "";
        }
      }
      for (var e: dynamic in goal)
      {
        read(e);
        if ((e == "-"))
        {
          e = "";
        }
      }
      var d1: dynamic = bfs(3, lines, exchanges);
      var d2: dynamic = bfs(2, goal, exchanges);
      var ans: dynamic = 6;
      for (var e1: dynamic in d2)
      {
        if ((ans <= e1.second))
        {
          continue;
        }
        if (d1.count(e1.first))
        {
          chmin(ans, (e1.second + d1.at(e1.first)));
        }
      }
      write(ans, "\n");
    }
  }
  return EXIT_SUCCESS;
}
