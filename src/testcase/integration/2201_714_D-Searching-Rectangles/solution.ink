// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var side: dynamic = cpp_array(2, 4);

func request(mid: dynamic, s: dynamic) -> dynamic
{
  var __cpp_switch_1: dynamic = s;
  if (__cpp_switch_1 == 0)
  {
    printf("? %d %d %d %d\n", mid, 1, n, n);
    break;
  }
  else if (__cpp_switch_1 == 1)
  {
    printf("? %d %d %d %d\n", 1, 1, mid, n);
    break;
  }
  else if (__cpp_switch_1 == 2)
  {
    printf("? %d %d %d %d\n", 1, mid, n, n);
    break;
  }
  else if (__cpp_switch_1 == 3)
  {
    printf("? %d %d %d %d\n", 1, 1, n, mid);
    break;
  }
  fflush(stdout);
  var tmp: dynamic = cpp_uninitialized();
  scanf("%d", (&tmp));
  return tmp;
}

func binSearch(val: dynamic, s: dynamic) -> dynamic
{
  var l: dynamic = 1;
  var r: dynamic = n;
  var mid: dynamic = cpp_uninitialized();
  if (((s % 2) == 0))
  {
    while ((l <= r))
    {
      mid = (((l + r)) / 2);
      if ((request(mid, s) >= val))
      {
        l = (mid + 1);
      } else
      {
        r = (mid - 1);
      }
    }
    return (l - 1);
  } else
  {
    while ((l <= r))
    {
      mid = (((l + r)) / 2);
      if ((request(mid, s) >= val))
      {
        r = (mid - 1);
      } else
      {
        l = (mid + 1);
      }
    }
    return l;
  }
}

func findSide(s: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 2))
    {
      side[s][i] = binSearch((i + 1), s);
      i += 1;
    }
  }
}

func exsists(xs1: dynamic, xe1: dynamic, ys1: dynamic, ye1: dynamic) -> dynamic
{
  return (((xs1 <= xe1) && (ys1 <= ye1)));
}

func intersect(xs1: dynamic, xe1: dynamic, ys1: dynamic, ye1: dynamic, xs2: dynamic, xe2: dynamic, ys2: dynamic, ye2: dynamic) -> dynamic
{
  var xs: dynamic = max(xs1, xs2);
  var xe: dynamic = min(xe1, xe2);
  var ys: dynamic = max(ys1, ys2);
  var ye: dynamic = min(ye1, ye2);
  return (((xs <= xe) && (ys <= ye)));
}

func check(xs1: dynamic, xe1: dynamic, ys1: dynamic, ye1: dynamic, xs2: dynamic, xe2: dynamic, ys2: dynamic, ye2: dynamic) -> dynamic
{
  printf("? %d %d %d %d\n", xs1, ys1, xe1, ye1);
  fflush(stdout);
  var tmp: dynamic = cpp_uninitialized();
  scanf("%d", (&tmp));
  if ((tmp != 1))
  {
    return false;
  }
  printf("? %d %d %d %d\n", xs2, ys2, xe2, ye2);
  fflush(stdout);
  scanf("%d", (&tmp));
  return ((tmp == 1));
}

class rect
{
  var xs: dynamic = cpp_uninitialized();
  var xe: dynamic = cpp_uninitialized();
  var ys: dynamic = cpp_uninitialized();
  var ye: dynamic = cpp_uninitialized();
  func rect() -> dynamic
  {
    }
  func rect(xs: dynamic, xe: dynamic, ys: dynamic, ye: dynamic) -> dynamic
  {
      self->xs = cpp_construct(xs);
      self->xe = cpp_construct(xe);
      self->ys = cpp_construct(ys);
      self->ye = cpp_construct(ye);
    }
  func field() -> dynamic
  {
      return ((((xe - xs) + 1)) * cpp_cast((((ye - ys) + 1))));
    }
}

func Solvex(xs1: dynamic, xe1: dynamic, xs2: dynamic, xe2: dynamic) -> dynamic
{
  var a1: dynamic = cpp_uninitialized();
  var a2: dynamic = cpp_uninitialized();
  var t1: dynamic = cpp_uninitialized();
  var t2: dynamic = cpp_uninitialized();
  var answt: dynamic = (1000000000 * 1000000000);
  {
    var i: dynamic = 0;
    while ((i < 2))
    {
      {
        var j: dynamic = 0;
        while ((j < 2))
        {
          var ys1: dynamic = side[2][i];
          var ys2: dynamic = side[2][(((i + 1)) % 2)];
          var ye1: dynamic = side[3][j];
          var ye2: dynamic = side[3][(((j + 1)) % 2)];
          if ((exsists(xs1, xe1, ys1, ye1) && exsists(xs2, xe2, ys2, ye2)))
          {
            if ((!intersect(xs1, xe1, ys1, ye1, xs2, xe2, ys2, ye2)))
            {
              if (check(xs1, xe1, ys1, ye1, xs2, xe2, ys2, ye2))
              {
                t1 = rect(xs1, xe1, ys1, ye1);
                t2 = rect(xs2, xe2, ys2, ye2);
                if (((t1.field() + t2.field()) < answt))
                {
                  answt = (t1.field() + t2.field());
                  a1 = t1;
                  a2 = t2;
                }
              }
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("! %d %d %d %d %d %d %d %d\n", a1.xs, a1.ys, a1.xe, a1.ye, a2.xs, a2.ys, a2.xe, a2.ye);
}

func Solvey(ys1: dynamic, ye1: dynamic, ys2: dynamic, ye2: dynamic) -> dynamic
{
  var a1: dynamic = cpp_uninitialized();
  var a2: dynamic = cpp_uninitialized();
  var t1: dynamic = cpp_uninitialized();
  var t2: dynamic = cpp_uninitialized();
  var answt: dynamic = (1000000000 * 1000000000);
  {
    var i: dynamic = 0;
    while ((i < 2))
    {
      {
        var j: dynamic = 0;
        while ((j < 2))
        {
          var xs1: dynamic = side[0][i];
          var xs2: dynamic = side[0][(((i + 1)) % 2)];
          var xe1: dynamic = side[1][j];
          var xe2: dynamic = side[1][(((j + 1)) % 2)];
          if ((exsists(xs1, xe1, ys1, ye1) && exsists(xs2, xe2, ys2, ye2)))
          {
            if ((!intersect(xs1, xe1, ys1, ye1, xs2, xe2, ys2, ye2)))
            {
              if (check(xs1, xe1, ys1, ye1, xs2, xe2, ys2, ye2))
              {
                t1 = rect(xs1, xe1, ys1, ye1);
                t2 = rect(xs2, xe2, ys2, ye2);
                if (((t1.field() + t2.field()) < answt))
                {
                  answt = (t1.field() + t2.field());
                  a1 = t1;
                  a2 = t2;
                }
              }
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("! %d %d %d %d %d %d %d %d\n", a1.xs, a1.ys, a1.xe, a1.ye, a2.xs, a2.ys, a2.xe, a2.ye);
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      findSide(i);
      sort(side[i], (side[i] + 2));
      i += 1;
    }
  }
  if ((side[1][0] < side[0][1]))
  {
    var xs1: dynamic = cpp_uninitialized();
    var xe1: dynamic = cpp_uninitialized();
    xs1 = side[0][0];
    xe1 = side[1][0];
    var xs2: dynamic = cpp_uninitialized();
    var xe2: dynamic = cpp_uninitialized();
    xs2 = side[0][1];
    xe2 = side[1][1];
    Solvex(xs1, xe1, xs2, xe2);
  } else
  {
    var ys1: dynamic = cpp_uninitialized();
    var ye1: dynamic = cpp_uninitialized();
    ys1 = side[2][0];
    ye1 = side[3][0];
    var ys2: dynamic = cpp_uninitialized();
    var ye2: dynamic = cpp_uninitialized();
    ys2 = side[2][1];
    ye2 = side[3][1];
    Solvey(ys1, ye1, ys2, ye2);
  }
  return 0;
}
