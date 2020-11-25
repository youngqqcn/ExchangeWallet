import XLSX from 'xlsx'
const OutExcel = function (data, head, option) {
  if (!(this instanceof OutExcel)) return new OutExcel(data, head);
  // this.init();
  this.head = head
  this.option = option
  this.toTable(data,(d)=>{
  })
};
OutExcel.prototype = {
  template (table){
    this.table = table
    // console.log(table)
    return `<html lang="ch" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:x="urn:schemas-microsoft-com:office:excel" xmlns="http://www.w3.org/TR/REC-html40"><meta http-equiv="content-type" content="application/vnd.ms-excel; charset=UTF-8"><head><!--[if gte mso 9]><xml><x:ExcelWorkbook><x:ExcelWorksheets><x:ExcelWorksheet><x:Name>{worksheet}</x:Name><x:WorksheetOptions><x:DisplayGridlines/></x:WorksheetOptions></x:ExcelWorksheet></x:ExcelWorksheets></x:ExcelWorkbook></xml><![endif]--></head><body>
              <table>${table}</table>
            </body></html>`
  },
  toTable (data) {
    let headTemplate = ''
    let contentTemplate = ''
    for(let key in this.head){
      if(this.head.hasOwnProperty(key)){
        headTemplate += this.headToHTML(key)
      }
    }
    headTemplate = `<tr>${headTemplate}</tr>`
    for(let i = 0;i<data.length;i++){
      let M = ''
      for(let key in this.head){
        if(data[i].hasOwnProperty(key)){
          M += this.contentToHtml(data[i][key])
        }
      }
      contentTemplate += `<tr>${M}</tr>`
    }
    // console.log(this.template(headTemplate+contentTemplate))
    // let blobFile = new Blob([this.template(headTemplate+contentTemplate)],{type: "application/vnd.ms-excel"})
    // let fileUrl = URL.createObjectURL(blobFile);
    // console.log(fileUrl)
    var xlsxParam = { raw: true };//转换成excel时，使用原始的格式
    let tableToEl = document.createElement('table')
    tableToEl.innerHTML = (headTemplate+contentTemplate)
    var wb = XLSX.utils.table_to_book(tableToEl,xlsxParam);
    var fileExcel = XLSX.write(wb, {
      bookType: "xlsx",
      bookSST: true,
      type: "array"
    });
    let blobFile = new Blob([fileExcel], { type: "application/octet-stream;charset=utf-8" })
    let fileUrl = URL.createObjectURL(blobFile);
    console.log(fileUrl)
    this.exInfo = {
      fileUrl:fileUrl
    }
  },
  headToHTML(d){
    return `<th>${this.head[d]}</th>`
  },
  contentToHtml(d){
    return `<td>${d}</td>`
  }
}
const MyExcel = {}
MyExcel.install =  (Vue, options) => {
  Vue.prototype.$excel = OutExcel
}
export default MyExcel
