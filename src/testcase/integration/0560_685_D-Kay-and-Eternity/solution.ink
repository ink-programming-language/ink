// Translated from solution.cpp.

class POINT
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  func POINT(x: dynamic = 0, y: dynamic = 0, v: dynamic = 0) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
      self->v = cpp_construct(v);
    }
  func operator_less(other: dynamic) -> dynamic
  {
      if ((self->x != other.x))
      {
        return (self->x < other.x);
      }
      if ((self->y != other.y))
      {
        return (self->y < other.y);
      }
      if ((self->v != other.v))
      {
        return (self->v < other.v);
      }
    }
}

var N: dynamic = cpp_uninitialized();

var K: dynamic = cpp_uninitialized();

var pt: dynamic = cpp_array((100013 * 3));

var ans: dynamic = cpp_array(100013);

var tmpans: dynamic = cpp_array((100013 * 2));

var num: dynamic = cpp_array((100013 * 2));

var mapy: dynamic = cpp_array((100013 * 2));

var loc: dynamic = cpp_array((100013 * 2));

func cmpx(a: dynamic, b: dynamic) -> dynamic
{
  return (a.x < b.x);
}

func cmpy(a: dynamic, b: dynamic) -> dynamic
{
  return (a.y < b.y);
}

func solve(k: dynamic, x: dynamic) -> dynamic
{
  var y: dynamic = pt[k].y;
  var S: dynamic = 0;
  while (((y >= 0) && ((mapy[pt[k].y] - mapy[y]) < K)))
  {
    S += num[cpp_update(y, "--")];
  }
  y += 1;
  var up: dynamic = pt[k].y;
  var down: dynamic = y;
  while ((down <= pt[k].y))
  {
    ans[S] += (cpp_cast(((mapy[(up + 1)] - mapy[up]))) * ((x - loc[up])));
    loc[up] = x;
    up += 1;
    S += num[up];
    while (((mapy[up] - mapy[down]) >= K))
    {
      S -= num[cpp_update(down, "++")];
    }
  }
}

func main() -> dynamic
{
  scanf("%d%d", (&N), (&K));
  srand(time(0));
  {
    var i: dynamic = int_cpp(0);
    while ((i < int_cpp(N)))
    {
      scanf("%d%d", (&pt[i].x), (&pt[i].y));
      pt[(i + N)].x = (pt[i].x + K);
      pt[(i + N)].y = pt[i].y;
      pt[i].v = 1;
      pt[(i + N)].v = -1;
      pt[(i + (2 * N))].y = (pt[i].y + K);
      i += 1;
    }
  }
  sort(pt, (pt + (3 * N)), cmpy);
  var k: dynamic = 0;
  var last: dynamic = pt[0].y;
  mapy[0] = pt[0].y;
  {
    var i: dynamic = int_cpp(0);
    while ((i < int_cpp((3 * N))))
    {
      if ((pt[i].y > last))
      {
        k += 1;
        mapy[k] = pt[i].y;
        last = pt[i].y;
      }
      pt[i].y = k;
      i += 1;
    }
  }
  mapy[(k + 1)] = 0x7fffffff;
  k = (3 * N);
  {
    var i: dynamic = int_cpp(0);
    while ((i < int_cpp((2 * N))))
    {
      while ((pt[i].v == 0))
      {
        pt[i] = pt[cpp_update(k, "--")];
      }
      i += 1;
    }
  }
  sort(pt, (pt + (2 * N)), cmpx);
  k = 0;
  last = pt[0].x;
  var head: dynamic = 0;
  {
    var i: dynamic = int_cpp(0);
    while ((i < int_cpp((2 * N))))
    {
      if ((pt[i].x > last))
      {
        k += 1;
        last = pt[i].x;
        {
          var j: dynamic = int_cpp(head);
          while ((j < int_cpp(i)))
          {
            num[pt[j].y] += pt[j].v;
            j += 1;
          }
        }
        head = i;
      }
      solve(i, pt[i].x);
      i += 1;
    }
  }
  {
    var i: dynamic = int_cpp(1);
    while ((i < int_cpp((N + 1))))
    {
      printf("%I64d%c", ans[i], " \n"[(i == N)]);
      i += 1;
    }
  }
  return 0;
}
