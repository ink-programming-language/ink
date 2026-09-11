// Translated from solution.cpp.

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Point(x: dynamic = 0, y: dynamic = 0) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
    }
}

class State
{
  var p: dynamic = cpp_uninitialized();
  var num: dynamic = cpp_uninitialized();
  func State(p: dynamic, num: dynamic) -> dynamic
  {
      self->p = cpp_construct(p);
      self->num = cpp_construct(num);
    }
}

var pdx: dynamic = [1, 0, -1, -2, -2, -2, -1, 0, 1, 2, 2, 2];

var pdy: dynamic = [-2, -2, -2, -1, 0, 1, 2, 2, 2, 1, 0, -1];

var sdx: dynamic = [0, 1, 0, -1, -1, -1, 0, 1, 1];

var sdy: dynamic = [0, -1, -1, -1, 0, 1, 1, 1, 0];

func main() -> dynamic
{
  var px: dynamic = cpp_uninitialized();
  var py: dynamic = cpp_uninitialized();
  while (((cin >> px) >> py))
  {
    if ((((px | py)) == 0))
    {
      break;
    }
    var n: dynamic = cpp_uninitialized();
    read(n);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var sx: dynamic = cpp_uninitialized();
        var sy: dynamic = cpp_uninitialized();
        read(sx, sy);
        {
          var j: dynamic = 0;
          while ((j < 9))
          {
            spp[i].push_back(Point((sx + sdx[j]), (sy + sdy[j])));
            j += 1;
          }
        }
        i += 1;
      }
    }
    var ok: dynamic = false;
    var que: dynamic = cpp_uninitialized();
    que.push(State(Point(px, py), -1));
    while ((!que.empty()))
    {
      var st: dynamic = que.front();
      que.pop();
      if ((st.num == (n - 1)))
      {
        ok = true;
        break;
      }
      {
        var i: dynamic = 0;
        while ((i < 12))
        {
          var p: dynamic = cpp_construct((st.p.x + pdx[i]), (st.p.y + pdy[i]));
          if (((((p.x < 0) || (9 < p.x)) || (p.y < 0)) || (9 < p.y)))
          {
            i += 1;
            continue;
          }
          {
            var j: dynamic = 0;
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
