// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var a: dynamic = cpp_construct((m + 1));
  for (var v: dynamic in a)
  {
    read(v);
  }
  for (var v: dynamic in b)
  {
    read(v);
  }
  var res: dynamic = 0;
  var state: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      state.push_back([[a[i], a[(i + 1)]], ((1 << b[i]))]);
      i += 1;
    }
  }
  {
    var rank: dynamic = n;
    while ((rank >= 0))
    {
      if ((rank == 0))
      {
        if ((!((state[0].second & 1))))
        {
          res += 1;
        }
        break;
      }
      var nex: dynamic = cpp_uninitialized();
      {
        var i: dynamic = 0;
        while ((i < state.size()))
        {
          var lef: dynamic = (state[i].first.first + (state[i].first.first % 2));
          var rig: dynamic = (state[i].first.second - (state[i].first.second % 2));
          if ((lef < rig))
          {
            if ((!((state[i].second & ((1 << rank))))))
            {
              res += (((rig - lef)) / 2);
            }
            nex.push_back([[(lef / 2), (rig / 2)], state[i].second]);
          }
          if ((rig != state[i].first.second))
          {
            var val: dynamic = state[(i + 1)].second;
            if ((state[i].second & ((1 << rank))))
            {
              if ((val & ((1 << rank))))
              {
                nex.push_back([[(rig / 2), ((rig / 2) + 1)], (val | state[i].second)]);
              } else
              {
                nex.push_back([[(rig / 2), ((rig / 2) + 1)], val]);
              }
            } else if ((val & ((1 << rank))))
            {
              nex.push_back([[(rig / 2), ((rig / 2) + 1)], state[i].second]);
            } else
            {
              res += 1;
              nex.push_back([[(rig / 2), ((rig / 2) + 1)], (state[i].second | val)]);
            }
          }
          i += 1;
        }
      }
      state = nex;
      rank -= 1;
    }
  }
  write(res, "\n");
  return 0;
}
