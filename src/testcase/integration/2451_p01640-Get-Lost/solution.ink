// Translated from solution.cpp.

var MAX = 100000;

var w: dynamic;

var h: dynamic;

var gx: dynamic;

var gy: dynamic;

var n: dynamic;

var input: dynamic;

var vec = cpp_array((MAX + 1), 4);

var r: dynamic;

func check(x: dynamic, y: dynamic, d: dynamic)
{
  var f = false;
  {
    var i = 0;
    while ((i <= r.size()))
    {
      var nx: dynamic;
      var ny: dynamic;
      if ((d == 0))
      {
        nx = x;
        ny = (*(lower_bound(vec[0][x].begin(), vec[0][x].end(), (-y))));
        ny = (-ny);
        if (((nx == gx) && (ny == gy)))
        {
          return true;
        }
        ny = (ny + 1);
        if ((((nx == gx) && (ny <= gy)) && (gy <= y)))
        {
          return true;
        }
      }
      if ((d == 1))
      {
        nx = (*(lower_bound(vec[1][y].begin(), vec[1][y].end(), x)));
        ny = y;
        if (((nx == gx) && (ny == gy)))
        {
          return true;
        }
        nx = (nx - 1);
        if ((((x <= gx) && (gx <= nx)) && (ny == gy)))
        {
          return true;
        }
      }
      if ((d == 2))
      {
        nx = x;
        ny = (*(lower_bound(vec[2][x].begin(), vec[2][x].end(), y)));
        if (((nx == gx) && (ny == gy)))
        {
          return true;
        }
        ny = (ny - 1);
        if ((((nx == gx) && (y <= gy)) && (gy <= ny)))
        {
          return true;
        }
      }
      if ((d == 3))
      {
        nx = (*(lower_bound(vec[3][y].begin(), vec[3][y].end(), (-x))));
        ny = y;
        nx = (-nx);
        if (((nx == gx) && (ny == gy)))
        {
          return true;
        }
        nx = (nx + 1);
        if ((((nx <= gx) && (gx <= x)) && (ny == gy)))
        {
          return true;
        }
      }
      x = nx;
      y = ny;
      if ((i == r.size()))
      {
        break;
      }
      if ((r[i] == cpp_char("R")))
      {
        d = (((d + 1)) % 4);
      } else
      {
        d = ((((d - 1) + 4)) % 4);
      }
      i += 1;
    }
  }
  return false;
}

func solve()
{
  var res = 0;
  {
    var i = 0;
    while ((i <= MAX))
    {
      {
        var j = 0;
        while ((j < 4))
        {
          vec[j][i].clear();
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < input.size()))
    {
      vec[1][input[i].second].push_back(input[i].first);
      vec[3][input[i].second].push_back(input[i].first);
      i += 1;
    }
  }
  vec[1][gy].push_back(gx);
  vec[3][gy].push_back(gx);
  {
    var i = 0;
    while ((i < input.size()))
    {
      vec[0][input[i].first].push_back(input[i].second);
      vec[2][input[i].first].push_back(input[i].second);
      i += 1;
    }
  }
  vec[0][gx].push_back(gy);
  vec[2][gx].push_back(gy);
  {
    var i = 0;
    while ((i < 4))
    {
      {
        var j = 1;
        while ((j <= MAX))
        {
          vec[i][j].push_back(0);
          vec[i][j].push_back(((if (((i % 2) == 0)) h else w) + 1));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= MAX))
    {
      {
        var j = 0;
        while ((j < vec[0][i].size()))
        {
          vec[0][i][j] = (-vec[0][i][j]);
          j += 1;
        }
      }
      {
        var j = 0;
        while ((j < vec[3][i].size()))
        {
          vec[3][i][j] = (-vec[3][i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= MAX))
    {
      {
        var j = 0;
        while ((j < 4))
        {
          if ((vec[j][i].size() > 0))
          {
            sort(vec[j][i].begin(), vec[j][i].end());
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var d = 0;
    while ((d < 4))
    {
      var upper = (if (((d % 2) == 0)) w else h);
      {
        var i = 1;
        while ((i <= upper))
        {
          {
            var j = 1;
            while ((j < vec[d][i].size()))
            {
              if (((d % 2) == 1))
              {
                var sx = (vec[d][i][j] - 1);
                if ((d == 3))
                {
                  sx = (-sx);
                }
                if (((sx <= 0) || (sx > w)))
                {
                  j += 1;
                  continue;
                }
                if (((((vec[d][i][j] - vec[d][i][(j - 1)]) - 1) > 0) && check(sx, i, d)))
                {
                  res += ((vec[d][i][j] - vec[d][i][(j - 1)]) - 1);
                }
              }
              if (((d % 2) == 0))
              {
                var sy = (vec[d][i][j] - 1);
                if ((d == 0))
                {
                  sy = (-sy);
                }
                if (((sy <= 0) || (sy > h)))
                {
                  j += 1;
                  continue;
                }
                if (((((vec[d][i][j] - vec[d][i][(j - 1)]) - 1) > 0) && check(i, sy, d)))
                {
                  res += ((vec[d][i][j] - vec[d][i][(j - 1)]) - 1);
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      d += 1;
    }
  }
  return res;
}

func main()
{
  read(w, h, gx, gy, n);
  {
    var i = 0;
    while ((i < n))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      input.push_back(P(x, y));
      i += 1;
    }
  }
  read(r);
  write((solve() + 4), "\n");
}
