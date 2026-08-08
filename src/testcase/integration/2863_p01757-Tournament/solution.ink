// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var a = cpp_construct((m + 1));
  for (var v in a)
  {
    read(v);
  }
  for (var v in b)
  {
    read(v);
  }
  var res = 0;
  var state: dynamic;
  {
    var i = 0;
    while ((i < m))
    {
      state.push_back([[a[i], a[(i + 1)]], ((1 << b[i]))]);
      i += 1;
    }
  }
  {
    var rank = n;
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
      var nex: dynamic;
      {
        var i = 0;
        while ((i < state.size()))
        {
          var lef = (state[i].first.first + (state[i].first.first % 2));
          var rig = (state[i].first.second - (state[i].first.second % 2));
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
            var val = state[(i + 1)].second;
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
