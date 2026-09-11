// Translated from solution.cpp.

func main() -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  read(k);
  var lun: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i < 10))
    {
      lun.push(i);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < k))
    {
      var x: dynamic = lun.front();
      lun.pop();
      if (((x % 10) != 0))
      {
        lun.push((((x * 10) + (x % 10)) - 1));
      }
      lun.push(((x * 10) + (x % 10)));
      if (((x % 10) != 9))
      {
        lun.push((((x * 10) + (x % 10)) + 1));
      }
      i += 1;
    }
  }
  write(lun.front(), "\n");
}
