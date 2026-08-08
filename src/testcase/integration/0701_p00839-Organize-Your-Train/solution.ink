// Translated from solution.cpp.

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func rolling_hash(s: dynamic)
{
  var base = 1000000007;
  var res = 0;
  for (var c in s)
  {
    res = ((res * base) + c);
  }
  return res;
}

func to_str(state: dynamic)
{
  var res = "";
  for (var s in state)
  {
    res += (s + ":");
  }
  return res;
}

func bfs(limit: dynamic, start: dynamic, exchanges: dynamic)
{
  var res: dynamic;
  var que: dynamic;
  res.insert([rolling_hash(to_str(start)), 0]);
  que.push(start);
  while ((!que.empty()))
  {
    var state = que.front();
    que.pop();
    var d = res.at(rolling_hash(to_str(state)));
    if ((d == limit))
    {
      break;
    }
    for (var element in exchanges)
    {
      var pos = element.first;
      var dir = element.second;
      var train = [move(state[pos.front()]), move(state[pos.back()])];
      {
        var i = 0;
        while ((i <= 1))
        {
          var from_cpp = i;
          var to = (1 - i);
          var p1 = pos[from_cpp];
          var p2 = pos[to];
          var d1 = dir[from_cpp];
          var d2 = dir[to];
          var t1 = train[from_cpp];
          var t2 = train[to];
          {
            var num = 1;
            while ((num <= t1.size()))
            {
              var tmp = (if (d1) t1.substr(0, num) else t1.substr((t1.size() - num)));
              if ((d1 == d2))
              {
                reverse(tmp.begin(), tmp.end());
              }
              state[p1] = (if (d1) t1.substr(num) else t1.substr(0, (t1.size() - num)));
              state[p2] = (if (d2) (tmp + t2) else (t2 + tmp));
              var h = rolling_hash(to_str(state));
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

func main()
{
  cin.tie(null);
  ios.sync_with_stdio(false);
  {
    var x: dynamic;
    var y: dynamic;
    while ((((cin >> x) >> y) && x))
    {
      var exchanges: dynamic;
      exchanges.reserve(y);
      {
        var i = 0;
        while ((i < y))
        {
          var a: dynamic;
          var b: dynamic;
          read(a, b);
          var p1 = (a[0] - cpp_char("0"));
          var d1 = ((a[1] == cpp_char("W")));
          var p2 = (b[0] - cpp_char("0"));
          var d2 = ((b[1] == cpp_char("W")));
          exchanges.emplace_back([p1, p2], [d1, d2]);
          i += 1;
        }
      }
      for (var e in lines)
      {
        read(e);
        if ((e == "-"))
        {
          e = "";
        }
      }
      for (var e in goal)
      {
        read(e);
        if ((e == "-"))
        {
          e = "";
        }
      }
      var d1 = bfs(3, lines, exchanges);
      var d2 = bfs(2, goal, exchanges);
      var ans = 6;
      for (var e1 in d2)
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
