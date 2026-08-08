// Translated from solution.cpp.

class Situation
{
  var oxigen: dynamic;
  var money: dynamic;
  var h: dynamic;
  var w: dynamic;
  var miningCnt: dynamic;
  var bitmask: dynamic;
  var visited: dynamic;
}

var mem = cpp_array(51, (1 << 10), 11, 11);

var field = cpp_array(11, 11);

var INF = 100000000;

var dy = [0, 0, 1];

var dx = [-1, 1, 0];

func solve()
{
  var h: dynamic;
  var w: dynamic;
  var f: dynamic;
  var m: dynamic;
  var o: dynamic;
  while ((((cin >> w) >> h) && (!(((w == 0) && (h == 0))))))
  {
    var q: dynamic;
    {
      var i = 0;
      while ((i < 11))
      {
        {
          var j = 0;
          while ((j < 11))
          {
            {
              var k = 0;
              while ((k < ((1 << 10))))
              {
                fill(mem[i][j][k], (mem[i][j][k] + 51), INF);
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    read(f, m, o);
    {
      var i = 0;
      while ((i < h))
      {
        {
          var j = 0;
          while ((j < w))
          {
            read(field[i][j]);
            field[i][j] = (-field[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < w))
      {
        var s: dynamic;
        s.money = 0;
        s.miningCnt = 1;
        s.oxigen = o;
        s.h = 0;
        s.w = i;
        s.bitmask = ((1 << i));
        s.visited.insert(make_pair(s.h, s.w));
        s.oxigen -= 1;
        if ((s.oxigen >= 1))
        {
          if ((field[s.h][s.w] > 0))
          {
            s.money += field[s.h][s.w];
          } else
          {
            s.oxigen = min(m, (s.oxigen - field[s.h][s.w]));
          }
          if (((s.money <= f) && (s.oxigen >= 1)))
          {
            mem[s.h][s.w][((1 << i))][s.oxigen] = s.money;
            q.push(s);
          }
        }
        i += 1;
      }
    }
    var minGoalCost = INF;
    while (q.size())
    {
      var s = q.front();
      q.pop();
      if (((s.h == (h - 1)) || (s.oxigen <= 1)))
      {
        if (((s.h == (h - 1)) && (s.oxigen >= 1)))
        {
          minGoalCost = min(minGoalCost, s.money);
        }
        continue;
      }
      {
        var i = 0;
        while ((i < 3))
        {
          var toH = (s.h + dy[i]);
          var toW = (s.w + dx[i]);
          var toCost = s.money;
          var toOx = (s.oxigen - 1);
          var toMining = s.miningCnt;
          if (((((toH >= 0) && (toW >= 0)) && (toH < h)) && (toW < w)))
          {
            if ((s.visited.find(make_pair(toH, toW)) == s.visited.end()))
            {
              if ((field[toH][toW] > 0))
              {
                toCost += field[toH][toW];
              } else
              {
                toOx = min(m, (toOx - field[toH][toW]));
              }
              toMining += 1;
            }
            var ns: dynamic;
            ns.h = toH;
            ns.w = toW;
            ns.money = toCost;
            ns.visited = s.visited;
            ns.oxigen = toOx;
            ns.visited.insert(make_pair(toH, toW));
            ns.miningCnt = toMining;
            ns.bitmask = s.bitmask;
            if ((i == 2))
            {
              ns.bitmask = ((1 << ns.w));
            } else
            {
              ns.bitmask |= ((1 << ns.w));
            }
            if ((((mem[toH][toW][ns.bitmask][toOx] > toCost) && (toCost <= f)) && (toCost <= minGoalCost)))
            {
              if ((toH != (h - 1)))
              {
                q.push(ns);
              } else
              {
                minGoalCost = min(minGoalCost, ns.money);
              }
              mem[toH][toW][ns.bitmask][toOx] = min(mem[toH][toW][ns.bitmask][toOx], toCost);
            }
          }
          i += 1;
        }
      }
    }
    if ((minGoalCost == INF))
    {
      write("NA", "\n");
    } else
    {
      write(minGoalCost, "\n");
    }
  }
}

func main()
{
  solve();
  return 0;
}
