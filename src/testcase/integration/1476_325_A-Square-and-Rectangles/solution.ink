// Translated from solution.cpp.

class coord
{
  var x1: dynamic;
  var x2: dynamic;
  var y1: dynamic;
  var y2: dynamic;
}

func main()
{
  var n: dynamic;
  read(n);
  var rect = cpp_array(n);
  var mnX = 31401;
  var mnY = 31401;
  var mxX = 0;
  var mxY = 0;
  var area = 0;
  {
    var i = 0;
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
  var x = (mxX - mnX);
  var y = (mxY - mnY);
  if (((x == y) && (area == (((1 * x) * y)))))
  {
    write("YES\n");
  } else
  {
    write("NO\n");
  }
  return 0;
}
