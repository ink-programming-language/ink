// Translated from solution.cpp.

var PI: dynamic = acos(-1.0);

var EPS: dynamic = 1.0e-10;

var INF: dynamic = (DBL_MAX / 1000);

func main() -> dynamic
{
  var tmp: dynamic = 0;
  {
    while (true)
    {
      var n: dynamic = cpp_uninitialized();
      var x: dynamic = cpp_uninitialized();
      read(n, x);
      if ((n == 0))
      {
        return 0;
      }
      var rStart: dynamic = cpp_uninitialized();
      var iStart: dynamic = cpp_uninitialized();
      var rGoal: dynamic = cpp_uninitialized();
      var iGoal: dynamic = cpp_uninitialized();
      read(rStart, iStart, rGoal, iGoal);
      {
        var j: dynamic = 0;
        while ((j < x))
        {
          read(r1[j], i1[j], r2[j], i2[j]);
          j += 1;
        }
      }
      var rIndex: dynamic = cpp_uninitialized();
      rIndex[1];
      {
        var j: dynamic = 0;
        while ((j < 3))
        {
          rIndex[((rStart - 1) + j)];
          rIndex[((rGoal - 1) + j)];
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j < x))
        {
          {
            var k: dynamic = 0;
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
      var m: dynamic = 0;
      var rPos: dynamic = cpp_uninitialized();
      {
        var it: dynamic = rIndex.begin();
        while ((it != rIndex.end()))
        {
          it->second = m;
          m += 1;
          rPos.push_back(it->first);
          it += 1;
        }
      }
      var damage: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 0;
        while ((j < x))
        {
          damage.insert(make_pair(make_pair(rIndex[r1[j]], i1[j]), make_pair(rIndex[r2[j]], i2[j])));
          damage.insert(make_pair(make_pair(rIndex[r2[j]], i2[j]), make_pair(rIndex[r1[j]], i1[j])));
          j += 1;
        }
      }
      var minDist: dynamic = cpp_construct(m, vector((n + 1), INF));
      minDist[rIndex[rStart]][iStart] = 0.0;
      var mm: dynamic = cpp_uninitialized();
      mm.insert(make_pair(0.0, make_pair(rIndex[rStart], iStart)));
      {
        while (true)
        {
          var dist0: dynamic = mm.begin()->first;
          var r0: dynamic = mm.begin()->second.first;
          var i0: dynamic = mm.begin()->second.second;
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
            var j: dynamic = 0;
            while ((j < 4))
            {
              var dist: dynamic = dist0;
              var r: dynamic = r0;
              var i: dynamic = i0;
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
