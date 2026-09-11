// Translated from solution.cpp.

var inf: dynamic = (1e9 + 7);

var big: dynamic = ((inf * 1) * inf);

var maxn: dynamic = 1e6;

var vert: dynamic = cpp_array(100010);

var hori: dynamic = cpp_array(100010);

var cntVert: dynamic = cpp_array(100010);

var cntHori: dynamic = cpp_array(100010);

var topBefore: dynamic = cpp_array(100010);

class Sofa
{
  var x1: dynamic = cpp_uninitialized();
  var y1: dynamic = cpp_uninitialized();
  var x2: dynamic = cpp_uninitialized();
  var y2: dynamic = cpp_uninitialized();
  func Sofa() -> dynamic
  {
    }
  func Sofa(a: dynamic, b: dynamic, c: dynamic, d: dynamic) -> dynamic
  {
      self->x1 = cpp_construct(a);
      self->y1 = cpp_construct(b);
      self->x2 = cpp_construct(c);
      self->y2 = cpp_construct(d);
    }
  func isVert() -> dynamic
  {
      return (x1 == x2);
    }
  func isHori() -> dynamic
  {
      return (y1 == y2);
    }
}

var sofas: dynamic = cpp_array(100010);

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var d: dynamic = cpp_uninitialized();
  read(d);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < d))
    {
      var x1: dynamic = cpp_uninitialized();
      var y1: dynamic = cpp_uninitialized();
      var x2: dynamic = cpp_uninitialized();
      var y2: dynamic = cpp_uninitialized();
      read(x1, y1, x2, y2);
      sofas[i] = Sofa(x1, y1, x2, y2);
      if ((x1 == x2))
      {
        vert[x1].push_back(i);
        hori[(min(y1, y2))].push_back(i);
        cntVert[x1] += 1;
      } else
      {
        hori[y1].push_back(i);
        vert[min(x1, x2)].push_back(i);
        cntHori[y1] += 1;
      }
      i += 1;
    }
  }
  {
    var row: dynamic = 1;
    while ((row <= m))
    {
      topBefore[row] = (topBefore[(row - 1)] + hori[row].size());
      row += 1;
    }
  }
  var cntl: dynamic = cpp_uninitialized();
  var cntr: dynamic = cpp_uninitialized();
  var cntt: dynamic = cpp_uninitialized();
  var cntb: dynamic = cpp_uninitialized();
  read(cntl, cntr, cntt, cntb);
  var toTheLeft: dynamic = 0;
  {
    var col: dynamic = 1;
    while ((col <= n))
    {
      if ((!vert[col].size()))
      {
        col += 1;
        continue;
      }
      for (var id: dynamic in vert[col])
      {
        var sofa: dynamic = sofas[id];
        var toTheRight: dynamic = cpp_uninitialized();
        var toTheTop: dynamic = cpp_uninitialized();
        var toTheBottom: dynamic = cpp_uninitialized();
        if (sofa.isVert())
        {
          toTheRight = ((d - toTheLeft) - cntVert[col]);
          toTheTop = (topBefore[min(sofa.y1, sofa.y2)] - 1);
          toTheBottom = ((((d - topBefore[min(sofa.y1, sofa.y2)]) + hori[min(sofa.y1, sofa.y2)].size()) - cntHori[min(sofa.y1, sofa.y2)]) - 1);
          if (cpp_binary(cpp_binary(cpp_binary((toTheLeft == cntl), "and", (toTheRight == cntr)), "and", (toTheTop == cntt)), "and", (toTheBottom == cntb)))
          {
            write((id + 1));
            return 0;
          }
        } else
        {
          toTheRight = (((d - toTheLeft) - cntVert[col]) - 1);
          toTheTop = (topBefore[sofa.y1] - hori[sofa.y1].size());
          toTheBottom = ((d - toTheTop) - cntHori[sofa.y1]);
          if (cpp_binary(cpp_binary(cpp_binary((cntl == ((toTheLeft + vert[col].size()) - 1)), "and", (cntr == toTheRight)), "and", (cntt == toTheTop)), "and", (cntb == toTheBottom)))
          {
            write((id + 1));
            return 0;
          }
        }
      }
      toTheLeft += vert[col].size();
      col += 1;
    }
  }
  write(-1);
  return 0;
}
