// Translated from solution.cpp.

var PI = acos(-1.0);

var EPS = 1.0e-10;

var INF = (DBL_MAX / 1000);

func main()
{
  var tmp = 0;
  {
    while (true)
    {
      var n: dynamic;
      var x: dynamic;
      read(n, x);
      if ((n == 0))
      {
        return 0;
      }
      var rStart: dynamic;
      var iStart: dynamic;
      var rGoal: dynamic;
      var iGoal: dynamic;
      read(rStart, iStart, rGoal, iGoal);
      {
        var j = 0;
        while ((j < x))
        {
          read(r1[j], i1[j], r2[j], i2[j]);
          j += 1;
        }
      }
      var rIndex: dynamic;
      rIndex[1];
      {
        var j = 0;
        while ((j < 3))
        {
          rIndex[((rStart - 1) + j)];
          rIndex[((rGoal - 1) + j)];
          j += 1;
        }
      }
      {
        var j = 0;
        while ((j < x))
        {
          {
            var k = 0;
            while ((k < 3))
            {
              rIndex[((r1[j] - 1) + k)];
              rIndex[((r2[j] - 1) + k)];
              k += 1;
            }
          }
          j += 1;
        }
      }
      if ((rIndex.begin()->first == 0))
      {
        rIndex.erase(rIndex.begin());
      }
      var m = 0;
      var rPos: dynamic;
      {
        var it = rIndex.begin();
        while ((it != rIndex.end()))
        {
          it->second = m;
          m += 1;
          rPos.push_back(it->first);
          it += 1;
        }
      }
      var damage: dynamic;
      {
        var j = 0;
        while ((j < x))
        {
          damage.insert(make_pair(make_pair(rIndex[r1[j]], i1[j]), make_pair(rIndex[r2[j]], i2[j])));
          damage.insert(make_pair(make_pair(rIndex[r2[j]], i2[j]), make_pair(rIndex[r1[j]], i1[j])));
          j += 1;
        }
      }
      var minDist = cpp_construct(m, vector((n + 1), INF));
      minDist[rIndex[rStart]][iStart] = 0.0;
      var mm: dynamic;
      mm.insert(make_pair(0.0, make_pair(rIndex[rStart], iStart)));
      {
        while (true)
        {
          var dist0 = mm.begin()->first;
          var r0 = mm.begin()->second.first;
          var i0 = mm.begin()->second.second;
          mm.erase(mm.begin());
          if ((dist0 > (minDist[r0][i0] + EPS)))
          {
            continue;
          }
          if (((r0 == rIndex[rGoal]) && (i0 == iGoal)))
          {
            printf("%.10f\n", dist0);
            break;
          }
          {
            var j = 0;
            while ((j < 4))
            {
              var dist = dist0;
              var r = r0;
              var i = i0;
              if (((j % 2) == 0))
              {
                r = ((r - 1) + j);
                if (((r == -1) || (r == m)))
                {
                  j += 1;
                  continue;
                }
                dist += abs((rPos[r] - rPos[r0]));
              } else
              {
                i = ((((((i - 3) + j) + n)) % n) + 1);
                dist += ((rPos[r] * sin((PI / n))) * 2.0);
              }
              if ((damage.find(make_pair(make_pair(r0, i0), make_pair(r, i))) != damage.end()))
              {
                j += 1;
                continue;
              }
              if ((dist < (minDist[r][i] - EPS)))
              {
                minDist[r][i] = dist;
                mm.insert(make_pair(dist, make_pair(r, i)));
              }
              j += 1;
            }
          }
        }
      }
    }
  }
}
