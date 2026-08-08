// Translated from solution.cpp.

func main()
{
  var k: dynamic;
  read(k);
  var lun: dynamic;
  {
    var i = 1;
    while ((i < 10))
    {
      lun.push(i);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < k))
    {
      var x = lun.front();
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
