// Translated from solution.cpp.

class Situation
{
  var oxigen: dynamic = cpp_uninitialized();
  var money: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var miningCnt: dynamic = cpp_uninitialized();
  var bitmask: dynamic = cpp_uninitialized();
  var visited: dynamic = cpp_uninitialized();
}

var mem: dynamic = cpp_array(51, (1 << 10), 11, 11);

var field: dynamic = cpp_array(11, 11);

var INF: dynamic = 100000000;

var dy: dynamic = [0, 0, 1];

var dx: dynamic = [-1, 1, 0];

func solve() -> dynamic
{
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var o: dynamic = cpp_uninitialized();
  while ((((cin >> w) >> h) && (!(((w == 0) && (h == 0))))))
  {
    var q: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < 11))
      {
        {
          var j: dynamic = 0;
          while ((j < 11))
          {
            {
              var k: dynamic = 0;
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
      var i: dynamic = 0;
      while ((i < h))
      {
        {
          var j: dynamic = 0;
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
      var i: dynamic = 0;
      while ((i < w))
      {
        var s: dynamic = cpp_uninitialized();
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
    var minGoalCost: dynamic = INF;
    while (q.size())
    {
      var s: dynamic = q.front();
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
        var i: dynamic = 0;
        while ((i < 3))
        {
          var toH: dynamic = (s.h + dy[i]);
          var toW: dynamic = (s.w + dx[i]);
          var toCost: dynamic = s.money;
          var toOx: dynamic = (s.oxigen - 1);
          var toMining: dynamic = s.miningCnt;
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
            var ns: dynamic = cpp_uninitialized();
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

func main() -> dynamic
{
  solve();
  return 0;
}
