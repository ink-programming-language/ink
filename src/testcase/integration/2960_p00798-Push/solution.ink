// Translated from solution.cpp.

var w: dynamic;

var h: dynamic;

var field = cpp_array(9, 9);

var dx = [-1, 0, 1, 0];

var dy = [0, -1, 0, 1];

func solve()
{
  var cargo = -1;
  var man = -1;
  {
    var i = 1;
    while ((i <= h))
    {
      {
        var j = 1;
        while ((j <= w))
        {
          if ((field[i][j] == 2))
          {
            cargo = ((i << 8) | j);
          } else if ((field[i][j] == 4))
          {
            man = ((i << 8) | j);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var st: dynamic;
  var dq: dynamic;
  dq.push_back(((cargo << 16) | man));
  st.insert(dq.front());
  dq.push_back(-1);
  var ans = 1;
  while ((dq.size() > 1))
  {
    var t = dq.front();
    dq.pop_front();
    if ((t < 0))
    {
      ans += 1;
      dq.push_back(t);
    } else
    {
      var mx = (t & 255);
      var my = (((t >> 8)) & 255);
      var cx = (((t >> 16)) & 255);
      var cy = (((t >> 24)) & 255);
      {
        var i = 0;
        while ((i < 4))
        {
          var mx2 = (mx + dx[i]);
          var my2 = (my + dy[i]);
          if (((mx2 == cx) && (my2 == cy)))
          {
            var cx2 = (cx + dx[i]);
            var cy2 = (cy + dy[i]);
            if ((field[cy2][cx2] == 3))
            {
              return ans;
            }
            if ((field[cy2][cx2] != 1))
            {
              var u = ((((cy2 << 24) | (cx2 << 16)) | (my2 << 8)) | mx2);
              if (st.insert(u).second)
              {
                dq.push_back(u);
              }
            }
          } else if ((field[my2][mx2] != 1))
          {
            var u = ((((cy << 24) | (cx << 16)) | (my2 << 8)) | mx2);
            if (st.insert(u).second)
            {
              dq.push_front(u);
            }
          }
          i += 1;
        }
      }
    }
  }
  return -1;
}

func main()
{
  while (cpp_comma(scanf("%d%d", (&w), (&h)), (w != 0)))
  {
    memset(field, 1, cpp_sizeof((field)));
    var d: dynamic;
    {
      var i = 1;
      while ((i <= h))
      {
        {
          var j = 1;
          while ((j <= w))
          {
            scanf("%d", (&d));
            field[i][j] = d;
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%d\n", solve());
  }
}
