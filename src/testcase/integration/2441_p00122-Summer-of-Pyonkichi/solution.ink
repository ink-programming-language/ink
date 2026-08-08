// Translated from solution.cpp.

class Point
{
  var x: dynamic;
  var y: dynamic;
  func Point(x: dynamic = 0, y: dynamic = 0)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
}

class State
{
  var p: dynamic;
  var num: dynamic;
  func State(p: dynamic, num: dynamic)
  {
      this->p = cpp_construct(p);
      this->num = cpp_construct(num);
    }
}

var pdx = [1, 0, -1, -2, -2, -2, -1, 0, 1, 2, 2, 2];

var pdy = [-2, -2, -2, -1, 0, 1, 2, 2, 2, 1, 0, -1];

var sdx = [0, 1, 0, -1, -1, -1, 0, 1, 1];

var sdy = [0, -1, -1, -1, 0, 1, 1, 1, 0];

func main()
{
  var px: dynamic;
  var py: dynamic;
  while (((cin >> px) >> py))
  {
    if ((((px | py)) == 0))
    {
      break;
    }
    var n: dynamic;
    read(n);
    {
      var i = 0;
      while ((i < n))
      {
        var sx: dynamic;
        var sy: dynamic;
        read(sx, sy);
        {
          var j = 0;
          while ((j < 9))
          {
            spp[i].push_back(Point((sx + sdx[j]), (sy + sdy[j])));
            j += 1;
          }
        }
        i += 1;
      }
    }
    var ok = false;
    var que: dynamic;
    que.push(State(Point(px, py), -1));
    while ((!que.empty()))
    {
      var st = que.front();
      que.pop();
      if ((st.num == (n - 1)))
      {
        ok = true;
        break;
      }
      {
        var i = 0;
        while ((i < 12))
        {
          var p = cpp_construct((st.p.x + pdx[i]), (st.p.y + pdy[i]));
          if (((((p.x < 0) || (9 < p.x)) || (p.y < 0)) || (9 < p.y)))
          {
            i += 1;
            continue;
          }
          {
            var j = 0;
            while ((j < spp[(st.num + 1)].size()))
            {
              if (((p.x == spp[(st.num + 1)][j].x) && (p.y == spp[(st.num + 1)][j].y)))
              {
                que.push(State(p, (st.num + 1)));
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
    }
    if (ok)
    {
      write("OK", "\n");
    } else
    {
      write("NA", "\n");
    }
  }
  return 0;
}
