// Translated from solution.cpp.

class coord
{
  var x1: dynamic = cpp_uninitialized();
  var x2: dynamic = cpp_uninitialized();
  var y1: dynamic = cpp_uninitialized();
  var y2: dynamic = cpp_uninitialized();
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var rect: dynamic = cpp_array(n);
  var mnX: dynamic = 31401;
  var mnY: dynamic = 31401;
  var mxX: dynamic = 0;
  var mxY: dynamic = 0;
  var area: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(rect[i].x1, rect[i].y1, rect[i].x2, rect[i].y2);
      area += ((1 * abs((rect[i].x1 - rect[i].x2))) * abs((rect[i].y1 - rect[i].y2)));
      mnX = min(mnX, rect[i].x1);
      mnY = min(mnY, rect[i].y1);
      mxX = max(mxX, rect[i].x2);
      mxY = max(mxY, rect[i].y2);
      i += 1;
    }
  }
  var x: dynamic = (mxX - mnX);
  var y: dynamic = (mxY - mnY);
  if (((x == y) && (area == (((1 * x) * y)))))
  {
    write("YES\n");
  } else
  {
    write("NO\n");
  }
  return 0;
}
